#pragma once
#include "ECLplus.h"

enum INS2100 {
    INS_PLAYER_POS,
    INS_PLAYER_KILL,
    INS_PLAYER_BOMB,
    INS_PLAYER_SET_LIVES,
    INS_PLAYER_SET_BOMBS
};

#define PlayerLivesPtr 0x004B5A40
#define PlayerLives Deref(PlayerLivesPtr)

#define PlayerBombsPtr 0x004B5A4C
#define PlayerBombs Deref(PlayerBombsPtr)

/* INS_2100 series: player manipulation */
BOOL ins_2100(ENEMY enm, INSTR* ins);
