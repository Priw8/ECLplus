#include "pch.h"
#include "intvar.h"
#include "ins2100.h"

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
        case INTVAR_LIVES:
            return PlayerLives;
        case INTVAR_BOMBS:
            return PlayerBombs;
        case INTVAR_GRAZE:
            return *INTVAR_GRAZE_LOC;
        case INTVAR_PIV:
            return *INTVAR_PIV_LOC;
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
        case INTVAR_LIVES:
        case INTVAR_BOMBS:
            EclPrint(
                "Warning: LIVES/BOMBS variables are read-only, due to issues with writing them directly. "
                "Use playerSetLives/playerSetBombs instructions instead.\n"
            );
            return &invalidAddr;
        case INTVAR_GRAZE:
            return INTVAR_GRAZE_LOC;
        case INTVAR_PIV:
            return INTVAR_PIV_LOC;
        default:
            return &invalidAddr;
    }
}
