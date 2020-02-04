#pragma once
#include "ECLplus.h"

enum INS2200 {
    INS_MSG_RESET,
    INS_MSG_RESET_CHANNEL,
    INS_MSG_SEND,
    INS_MSG_RECEIVE
};

typedef struct {
    FLOAT a;
    FLOAT b;
    FLOAT c;
    FLOAT d;
} MESSAGE;

typedef std::list<MESSAGE*> MESSAGELIST;

/* INS_2200 series: enemy communication */
BOOL ins_2200(ENEMY enm, INSTR* ins);
