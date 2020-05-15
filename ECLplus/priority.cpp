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

#define RegisterOnTickFunction ((void(__stdcall*)(UPDATE_FUNC*, DWORD effectivePriority)) 0x401140)
#define RegisterOnDrawFunction ((void(__stdcall*)(UPDATE_FUNC*, DWORD effectivePriority)) 0x4011f0)
#define OnTickEnemyFull ((INT(__thiscall*)(ENEMYFULL*)) 0x00420700)

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

static INT RebuildPriorities(PRIORITYMGR *mgr) {
	if (!GameEnmMgr) {
		return 1;
	}
	for (auto list = mgr->enemyLists.begin(); list != mgr->enemyLists.end(); ++list) {
		list->clear();
	}
	for (ENEMYLISTNODE* node = GameEnmMgr->head; node; node = node->next) {
		DWORD effectivePriority = GetExField(&node->obj->enm, targetEffPriority);
		
		auto& list = mgr->enemyLists[effectivePriority];
		auto newIter = list.insert(list.end(), node->obj);
		SetExField(&node->obj->enm, iterInRunGroup, newIter);
		SetExField(&node->obj->enm, currentEffPriority, effectivePriority);
	}
	return 1;
}
static INT __fastcall RebuildPriorities(LPVOID mgr) {
	return RebuildPriorities((PRIORITYMGR*)mgr);
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
		//EclPrintf("%d < %d\n", priority, priorityMgr.calleesBefore.size());
		//EclPrintf("%#x BF %d\n", priority, priorityMgr.calleesBefore[priority].EffectivePriority());
		//EclPrintf("%#x AF %d\n", priority, priorityMgr.calleesAfter[priority].EffectivePriority());

		// priorityMgr will be around forever so we can just leak the update funcs.
	}
	// Rebuild enemy lists after all other update funcs have run
	auto funcRebuild = new UPDATE_FUNC();
	funcRebuild->callee = &priorityMgr;
	funcRebuild->function = RebuildPriorities;
	RegisterOnTickFunction(funcRebuild, PRIORITY_MAX + 1);
}

VOID __stdcall RunEnemiesForEnemyManager() {
	RunEnemiesInRunGroup(&priorityMgr.calleeForEnemyManager);
}

VOID __stdcall RemoveEnemyFromRunGroup(ENEMYFULL* full) {
	DWORD effPriority = GetExField(&full->enm, currentEffPriority);
	EclPrintf("Priority %d\n", effPriority);
	if (effPriority) {
		auto iter = GetExField(&full->enm, iterInRunGroup);
		auto &list = priorityMgr.enemyLists[effPriority];
		EclPrintf("R -> %d\n", list.size());
		list.erase(iter);

		iter = {};
		SetExField(&full->enm, iterInRunGroup, iter);
		SetExField(&full->enm, currentEffPriority, 0);
	}
}

VOID __stdcall DebugStuffStuff(ENEMYFULL* full) {
	EclPrintf("%d %d run %8x%8x\n", TimeInStage, *(DWORD*)((CHAR*)full + 0x1498), *(DWORD*)((CHAR*)full + 0x528c), *(DWORD*)((CHAR*)full + 0x5290));

}
