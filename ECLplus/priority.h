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
#pragma once
#include "ECLplus.h"

// Prevent accidental usage of these windows macros, we don't need to use a printer >_>
#undef MIN_PRIORITY
#undef MAX_PRIORITY
// Min and max priorities that ECL is allowed to use.
#define PRIORITY_MIN 0x01
#define PRIORITY_MAX 0x25
// Priority of some notable things.
#define PRIORITY_ENEMY 0x1b
#define PRIORITY_GAMETHREAD 0x10
#define NUM_PRIORITIES (PRIORITY_MAX - PRIORITY_MIN + 1)

// "Effective priorities" are the true priorities we use interally.
#define EFFECTIVE_PRIORITY_DEFAULT (RUNGROUP { PRIORITY_ENEMY, REL_AFTER }.EffectivePriority())
#define EFFECTIVE_PRIORITY_MAINTHREAD (RUNGROUP::OfGameThread().EffectivePriority())
// Min and max values accessible by ECL
#define EFFECTIVE_PRIORITY_MAX (RUNGROUP { PRIORITY_MAX, REL_AFTER }.EffectivePriority())
#define EFFECTIVE_PRIORITY_MIN (RUNGROUP { PRIORITY_MIN, REL_BEFORE }.EffectivePriority())

enum PRIORITY_REL : INT {
	REL_BEFORE = -1,
	REL_DURING = 0,
	REL_AFTER = 1,
};

#pragma pack(push, 1)
struct RUNGROUP {
	DWORD priority;
	PRIORITY_REL rel;

	constexpr DWORD EffectivePriority() const {
		return (DWORD) (3 * (INT) priority + rel);
	}

	static RUNGROUP OfGameThread() {
		return { PRIORITY_GAMETHREAD, REL_DURING };
	}

	constexpr BOOL IsUi() const {
		DWORD gamePriority = RUNGROUP::OfGameThread().EffectivePriority();
		return this->EffectivePriority() < gamePriority;
	}

	constexpr BOOL IsWorld() const {
		return !this->IsUi();
	}
};
#pragma pack(pop)

// 0 is used to mean "no value" in some places
static_assert(PRIORITY_MIN > 0, "ambiguous use of priority 0");
static_assert(EFFECTIVE_PRIORITY_MIN > 0, "ambiguous use of priority 0");

struct UPDATE_FUNC;

#pragma pack(push, 1)
struct UPDATE_FUNC_LISTNODE {
	UPDATE_FUNC* entry;
	UPDATE_FUNC_LISTNODE* next;
	UPDATE_FUNC_LISTNODE* prev;
	UPDATE_FUNC_LISTNODE* unknown;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct UPDATE_FUNC {
	DWORD priority;
	DWORD flags;
	INT (__fastcall* function)(LPVOID);
	VOID (__fastcall* onRegistration)(LPVOID);
	INT (__fastcall* onCleanup)(LPVOID);
	UPDATE_FUNC_LISTNODE listNode;
	LPVOID callee;

	UPDATE_FUNC()
		: priority(0)
		, flags(3)
		, function(nullptr)
		, onRegistration(nullptr)
		, onCleanup(nullptr)
		, listNode { this, nullptr, nullptr, nullptr }
		, callee(nullptr)
	{}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct UPDATE_FUNC_REGISTRY {
	UPDATE_FUNC onTickListHead;
	UPDATE_FUNC onDrawListHead;
	UPDATE_FUNC* unknown;
	BOOL isCleaningUp;
};
#pragma pack(pop)

VOID InitPriorities();
VOID __stdcall RunEnemiesForEnemyManager();
VOID __stdcall RemoveEnemyFromRunGroup(ENEMYFULL* full);
VOID __stdcall DebugStuffStuff(ENEMYFULL* full);
