#pragma once
#include "pch.h"
#include "ECLplus.h"

enum {
    INTVAR_INPUT,
    INTVAR_SCORE,
    INTVAR_HISCORE,
    INTVAR_BOMBING
};
#define INTVAR_INPUT_LOC (DWORD*)0x004B3448
#define INTVAR_SCORE_LOC (DWORD*)0x004B59FC
#define INTVAR_HISCORE_LOC (DWORD*)0x004B59C0
#define INTVAR_BOMBING_VAL GetDwordField(Deref(0x004B7688), 0x30)

DWORD IntVarGetVal(ENEMY enm, DWORD var);

DWORD* IntVarGetAddr(ENEMY enm, DWORD var);
