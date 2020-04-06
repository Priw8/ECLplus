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
#include "intvar.h"
#include "ins2100.h"

/* Extra global integer variables */
static DWORD GI[4] = {0, 0, 0, 0};

DWORD IntVarGetVal(ENEMY* enm, DWORD var) {
    switch(var) {
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
            return GI[var - INTVAR_GI4];
        case INTVAR_IFRAMES:
            return GetDwordField(Player, PlayerIframeIntField);
        case INTVAR_PLAYER_STATE:
            return GetDwordField(Player, PlayerStateField);
        case INTVAR_HYPERTIMER:
            return *INTVAR_HYPERTIMER_LOC;
        case INTVAR_ISDIALOG:
            return *INTVAR_ISDIALOG_LOC;
        case INTVAR_SPELLBONUS:
            return GameSpell ? GameSpell->bonus : 0;
        default:
            return 0;
    }
}

static DWORD invalidAddr; /* Return a pointer to a valid variable to avoid crashing the game. */
DWORD* IntVarGetAddr(ENEMY* enm, DWORD var) {
    switch (var) {
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
            return &GI[var - INTVAR_GI4];
        case INTVAR_IFRAMES:
            return &invalidAddr;
        case INTVAR_PLAYER_STATE:
            return &GetDwordField(Player, PlayerStateField);
        case INTVAR_HYPERTIMER:
        case INTVAR_ISDIALOG:
        case INTVAR_SPELLBONUS:
            return &invalidAddr;
        default:
            return &invalidAddr;
    }
}
