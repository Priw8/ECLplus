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
#include "ECLplus.h"

enum INS2100 {
    INS_PLAYER_POS = 2100,
    INS_PLAYER_KILL,
    INS_PLAYER_BOMB,
    INS_PLAYER_SET_LIVES,
    INS_PLAYER_SET_BOMBS,
    INS_PLAYER_SET_POWER,
    INS_PLAYER_SET_IFRAMES,
    INS_PLAYER_TOGGLE_SHOT,
    INS_PLAYER_TOGGLE_BOMB,
    INS_PLAYER_SET_HYPER_TIMER
};

#define PlayerPtr 0x004B77D0
#define Player (Deref(PlayerPtr))
#define PlayerXField 0x61C
#define PlayerYField 0x620
#define PlayerStateField 0x18DB0
#define PlayerIframeIntField 0x18E7C
#define PlayerIframeFloatField 0x18E80

#define PlayerLivesPtr 0x004B5A40
#define PlayerLives Deref(PlayerLivesPtr)

#define PlayerBombsPtr 0x004B5A4C
#define PlayerBombs Deref(PlayerBombsPtr)

#define PlayerPowerPtr 0x004B5A30
#define PlayerPower Deref(PlayerPowerPtr)

#define PlayerHyperTimerPtr 0x004B5AAC
#define PlayerHyperTimer *(FLOAT*)PlayerHyperTimerPtr

/* INS_2100 series: player manipulation */
BOOL ins_2100(ENEMY* enm, INSTR* ins);
