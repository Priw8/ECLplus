#pragma once
#include "ECLplus.h"

enum INS2000 {
    INS_MSG_BOX,
    INS_PRINTF,
    INS_CLS,
    INS_DRAW_TEXT,
    INS_TEXT_COLOR,
    INS_BGM_SWITCH
};

/* INS_2000 series: general debug utilities */
BOOL ins_2000(ENEMY* enm, INSTR* ins);
