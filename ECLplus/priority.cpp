/*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce this list of conditions and the following disclaimer
*    in the documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
* OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "pch.h"
#include "ECLplus.h"
#include "ins2200.h"
#include "priority.h"
#include <cassert>

struct PRIORITYMGR {
	std::array<std::list<ENEMYFULL*>, EFFECTIVE_PRIORITY_MAX + 1> enemyLists;
	RUNGROUP calleeForEnemyManager;
	std::array<RUNGROUP, PRIORITY_MAX + 1> calleesBefore;
	std::array<RUNGROUP, PRIORITY_MAX + 1> calleesAfter;

	PRIORITYMGR()
		: enemyLists {}
		, calleeForEnemyManager { PRIORITY_ENEMY, REL_DURING }
	{
		auto iterBefore = this->calleesBefore.begin();
		auto iterAfter = this->calleesAfter.begin();
		for (DWORD prio = 0; prio <= PRIORITY_MAX; prio++) {
			*iterBefore++ = { prio, REL_BEFORE };
			*iterAfter++ = { prio, REL_AFTER };
		}
		assert(iterBefore == this->calleesBefore.end());
		assert(iterAfter == this->calleesAfter.end());
	}
};

static PRIORITYMGR priorityMgr;

#define RegisterOnTickFunction ((VOID(__stdcall*)(UPDATE_FUNC*, DWORD effectivePriority)) 0x401140)
#define RegisterOnDrawFunction ((VOID(__stdcall*)(UPDATE_FUNC*, DWORD effectivePriority)) 0x4011f0)
#define OnTickEnemyFull ((INT(__thiscall*)(ENEMYFULL*)) 0x00420700)
// the third stack argument to this was originally optimized out; we stick a starting priority in there
#define EnemyMgrCreateEnemy ((ENEMYFULL*(__thiscall*)(ENEMYMGR*, CHAR*, VOID*, DWORD effectivePriority)) 0x41e140)

VOID __stdcall InitEnemyExFields(ENEMYFULL* full, DWORD effPriority) {
	if (!effPriority) {
		EclMsg("Enemy created with default effective priority 0!  This is a bug in ECLplus!");
	}
	SetExField(&full->enm, defaultEffPriority, effPriority);
	// (for any other fields we keep the zeros from an earlier memset)
}

// Get an enemy's current effective priority; i.e. the effective priority of the enemy list that the enemy
// would run from on the next tick, assuming that it encounters no instruction to change priority before
// the end of this tick.
DWORD GetEnemyEffectivePriority(ENEMY* enemy) {
	// If this is the enemy's first tick, target might not yet be set.
	DWORD target = GetExField(enemy, targetEffPriority);
	DWORD def = GetExField(enemy, defaultEffPriority);
	return target ? target : def;
}

ENEMYFULL* __stdcall PatchedCreateChildEnemy(ENEMY* parent, CHAR* subname, void* instr, DWORD unusedArg3) {
	return EnemyMgrCreateEnemy(GameEnmMgr, subname, instr, GetEnemyEffectivePriority(parent));
}

ENEMYFULL* __stdcall PatchedCreateNonChildEnemy(CHAR* subname, void* instr, DWORD unusedArg3) {
	return EnemyMgrCreateEnemy(GameEnmMgr, subname, instr, EFFECTIVE_PRIORITY_DEFAULT);
}

VOID __stdcall AfterNewEnemyRunsFirstTick(ENEMYFULL* full) {
	if (GetExField(&full->enm, targetEffPriority)) {
		// The enemy explicitly set its priority on the first frame. Guarantee that it runs on the next frame.
		full->enm.flags &= ~FLAG_RAN_EARLY;
	}
	else {
		// No priority was explicitly set; simulate all of the vanilla game's run order bugs.
		//
		// Insert ourselves immediately into the rungroup list, and leave the EARLY_RUN flag on so that it gets
		// skipped when seen.
		SetExField(&full->enm, targetEffPriority, GetExField(&full->enm, defaultEffPriority));
		DWORD effectivePriority = GetExField(&full->enm, targetEffPriority);
		auto& list = priorityMgr.enemyLists[effectivePriority];
		list.insert(list.end(), full);
	}
	SetExField(&full->enm, defaultEffPriority, 0); // nothing should look at this field anymore
}

static INT RunEnemiesInRunGroup(RUNGROUP *group) {
	// TODO: Provide filters on GameThread flags to simulate how some other on_ticks work.
	//       (though I have no idea what any of the flags mean...)
	//if (group->IsWorld() && GameThread && (GameThread->flags & 0x407)) {
	//	return 1;
	//}

	auto &list = priorityMgr.enemyLists[group->EffectivePriority()];
	auto it = list.begin();
	while (it != list.end()) {
		ENEMYFULL* full = *it;

		// Reproduce a timing bug in the game by reading the `next` pointer a bit too early.
		// If we are at the last enemy, then this will produce `list.end()`, and therefore we
		// will stop iterating after this enemy even if it creates children.
		// (this causes the children to be skipped on the next tick, as we did not get the
		//  chance to clear their RAN_EARLY flag on this tick)
		auto next = ++it;

		if (full->enm.flags & FLAG_DELETEME || OnTickEnemyFull(full)) {
			full->vtable->Destroy(full, 1);
		} else {
			full->enm.flags &= ~FLAG_RAN_EARLY;
		}

		it = next;
	}
	return 1;
}
static INT __fastcall RunEnemiesInRunGroup(LPVOID group) {
	return RunEnemiesInRunGroup((RUNGROUP*)group);
}

VOID DebugEnemyCounts();

static INT RebuildPriorities(PRIORITYMGR *mgr, DWORD effPriorityFrom, DWORD effPriorityTo) {
	if (GameEnmMgr && GameEnmMgr->head) {
		DebugEnemyCounts();
	}

	if (!GameEnmMgr) {
		return 1;
	}

	for (DWORD i = effPriorityFrom; i <= effPriorityTo; ++i) {
		mgr->enemyLists[i].clear();
	}
	for (ENEMYLISTNODE* node = GameEnmMgr->head; node; node = node->next) {
		DWORD effPriority = GetExField(&node->obj->enm, targetEffPriority);
		if (effPriorityFrom <= effPriority && effPriority <= effPriorityTo) {
			auto& list = mgr->enemyLists[effPriority];
			list.insert(list.end(), node->obj);
		}
	}
	if (GameEnmMgr && GameEnmMgr->head) {
		DebugEnemyCounts();
	}
	return 1;
}
static INT __fastcall RebuildPrioritiesUi(LPVOID mgr) {
	return RebuildPriorities((PRIORITYMGR*)mgr, EFFECTIVE_PRIORITY_MIN_UI, EFFECTIVE_PRIORITY_MAX_UI);
}
static INT __fastcall RebuildPrioritiesWorld(LPVOID mgr) {
	return RebuildPriorities((PRIORITYMGR*)mgr, EFFECTIVE_PRIORITY_MIN_WORLD, EFFECTIVE_PRIORITY_MAX_WORLD);
}

VOID InitPriorities() {
	priorityMgr = {};

	for (DWORD priority = PRIORITY_MIN; priority <= PRIORITY_MAX; priority++) {
		auto funcBefore = new UPDATE_FUNC();
		auto funcAfter = new UPDATE_FUNC();

		funcBefore->callee = &priorityMgr.calleesBefore[priority];
		funcAfter->callee = &priorityMgr.calleesAfter[priority];

		funcBefore->function = RunEnemiesInRunGroup;
		funcAfter->function = RunEnemiesInRunGroup;

		RegisterOnTickFunction(funcBefore, priorityMgr.calleesBefore[priority].EffectivePriority());
		RegisterOnTickFunction(funcAfter, priorityMgr.calleesAfter[priority].EffectivePriority());

		// priorityMgr will be around forever so we can just leak the update funcs.
	}
	// Update funcs to rebuild enemy lists
	auto funcRebuildUi = new UPDATE_FUNC();
	funcRebuildUi->callee = &priorityMgr;
	funcRebuildUi->function = RebuildPrioritiesUi;
	RegisterOnTickFunction(funcRebuildUi, EFFECTIVE_PRIORITY_REBUILD_UI);

	auto funcRebuildWorld = new UPDATE_FUNC();
	funcRebuildWorld->callee = &priorityMgr;
	funcRebuildWorld->function = RebuildPrioritiesWorld;
	RegisterOnTickFunction(funcRebuildWorld, EFFECTIVE_PRIORITY_REBUILD_WORLD);
}

VOID __stdcall RunEnemiesForEnemyManager() {
	RunEnemiesInRunGroup(&priorityMgr.calleeForEnemyManager);
}

VOID __stdcall ClearAllEnemyLists() {
	auto& lists = priorityMgr.enemyLists;
	for (auto it = lists.begin(); it != lists.end(); ++it) {
		it->clear();
	}
}

VOID DebugEnemyCounts() {
	auto RelChar = [](PRIORITY_REL rel) -> CHAR {
		switch (rel) {
		case REL_BEFORE: return '-';
		case REL_DURING: return '=';
		case REL_AFTER: return '+';
		default: return '?';
		}
	};

	if (!GameEnmMgr) {
		return;
	}

	SIZE_T enemyCount = 0;
	for (auto node = GameEnmMgr->head; node; node = node->next) {
		enemyCount += 1;
	}

	EclPrintf(" TIME ALL");
	for (SIZE_T i = 0; i < priorityMgr.enemyLists.size(); i++) {
		if (!priorityMgr.enemyLists[i].empty()) {
			auto group = RUNGROUP::FromEffectivePriority(i);
			EclPrintf(" %02x%c", group.priority, RelChar(group.rel));
		}
	}
	EclPrintf("\n");

	EclPrintf("%5d %3d", TimeInStage, enemyCount);
	for (auto &list: priorityMgr.enemyLists) {
		if (!list.empty()) {
			EclPrintf(" %3d", list.size());
		}
	}
	EclPrintf("\n");
}

VOID __stdcall DebugStuffStuff(ENEMYFULL* full) {
	EclPrintf("%d %d run %8x%8x\n", TimeInStage, *(DWORD*)((CHAR*)full + 0x1498), *(DWORD*)((CHAR*)full + 0x528c), *(DWORD*)((CHAR*)full + 0x5290));
}
