#include "pch.h"
#include "intvar.h"
#include "ins2100.h"

/* Extra global integer variables */
static DWORD GI[4] = {0, 0, 0, 0};

DWORD IntVarGetVal(ENEMY* enm, DWORD var) {
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
        case INTVAR_CONTINUES:
            return *INTVAR_CONTINUES_LOC;
        case INTVAR_CREDITS:
            return *INTVAR_CREDITS_LOC;
        case INTVAR_GI4:
        case INTVAR_GI5:
        case INTVAR_GI6:
        case INTVAR_GI7:
            return GI[var + 8000 - INTVAR_GI4];
        case INTVAR_IFRAMES:
            return GetDwordField(Player, PlayerIframeIntField);
        case INTVAR_PLAYER_STATE:
            return GetDwordField(Player, PlayerStateField);
        default:
            return 0;
    }
}

static DWORD invalidAddr; /* Return a pointer to a valid variable to avoid crashing the game. */
DWORD* IntVarGetAddr(ENEMY* enm, DWORD var) {
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
        case INTVAR_CONTINUES:
            return INTVAR_CONTINUES_LOC;
        case INTVAR_CREDITS:
            return INTVAR_CREDITS_LOC;
        case INTVAR_GI4:
        case INTVAR_GI5:
        case INTVAR_GI6:
        case INTVAR_GI7:
            return &GI[var + 8000 - INTVAR_GI4];
        case INTVAR_IFRAMES:
            return &invalidAddr;
        case INTVAR_PLAYER_STATE:
            return &GetDwordField(Player, PlayerStateField);
        default:
            return &invalidAddr;
    }
}
