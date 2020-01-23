#include "pch.h"
#include "intvar.h"

DWORD IntVarGetVal(ENEMY enm, DWORD var) {
    switch(var + 8000) {
        case INTVAR_INPUT:
            return *INTVAR_INPUT_LOC;
        case INTVAR_SCORE:
            return *INTVAR_SCORE_LOC;
        case INTVAR_HISCORE:
            return *INTVAR_HISCORE_LOC;
        case INTVAR_BOMBING:
            return INTVAR_BOMBING_VAL;
        default:
            return 0;
    }
}

static DWORD invalidAddr; /* Return a pointer to a valid variable to avoid crashing the game. */
DWORD* IntVarGetAddr(ENEMY enm, DWORD var) {
    switch (var + 8000) {
        case INTVAR_INPUT:
            return INTVAR_INPUT_LOC;
        case INTVAR_SCORE:
            return INTVAR_SCORE_LOC;
        case INTVAR_HISCORE:
            return INTVAR_HISCORE_LOC;
        case INTVAR_BOMBING:
            EclPrint("Warning: variable BOMBING is read-only\n");
            return &invalidAddr;
        default:
            return &invalidAddr;
    }
}
