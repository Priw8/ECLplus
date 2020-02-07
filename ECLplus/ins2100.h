#pragma once
#include "ECLplus.h"

enum INS2100 {
    INS_PLAYER_POS,
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
