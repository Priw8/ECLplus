#pragma once
#include "ECLplus.h"

enum INS2100 {
    INS_PLAYER_POS,
    INS_PLAYER_KILL,
    INS_PLAYER_BOMB
};

/* INS_2100 series: player manipulation */
BOOL ins_2100(ENEMY enm, INSTR* ins);
