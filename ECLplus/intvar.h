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

#pragma once
#include "pch.h"
#include "ECLplus.h"

enum {
    INTVAR_INPUT = -8000,
    INTVAR_SCORE,
    INTVAR_HISCORE,
    INTVAR_BOMBING,
    INTVAR_LIVES,
    INTVAR_BOMBS,
    INTVAR_GRAZE,
    INTVAR_PIV,
    INTVAR_CONTINUES,
    INTVAR_CREDITS,
    INTVAR_GI4,
    INTVAR_GI5,
    INTVAR_GI6,
    INTVAR_GI7,
    INTVAR_IFRAMES,
    INTVAR_PLAYER_STATE,
    INTVAR_HYPERTIMER,
    INTVAR_ISDIALOG,
    INTVAR_SPELLBONUS,
    INTVAR_PRIORITY,
    INTVAR_PRIORITY_REL,
};
#define INTVAR_INPUT_LOC (DWORD*)0x004B3448
#define INTVAR_SCORE_LOC (DWORD*)0x004B59FC
#define INTVAR_HISCORE_LOC (DWORD*)0x004B59C0
#define INTVAR_BOMBING_VAL GetDwordField(Deref(0x004B7688), 0x30)
#define INTVAR_GRAZE_LOC (DWORD*)0x004B5A0C
#define INTVAR_PIV_LOC (DWORD*)0x004B5A24
#define INTVAR_CONTINUES_LOC (DWORD*)0x004B5A04
#define INTVAR_CREDITS_LOC (DWORD*)0x004B59D0
#define INTVAR_HYPERTIMER_LOC (DWORD*)0x004B5AA8
#define INTVAR_ISDIALOG_LOC (DWORD*)((*(DWORD*)0x004B76AC) + 0x1C0)

DWORD IntVarGetVal(ENEMY* enm, DWORD var);

DWORD* IntVarGetAddr(ENEMY* enm, DWORD var);
