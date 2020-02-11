#pragma once
#include "ECLplus.h"

enum INS2200 {
    INS_MSG_RESET,
    INS_MSG_RESET_CHANNEL,
    INS_MSG_SEND,
    INS_MSG_RECEIVE,
    INS_MSG_PEEK,
    INS_MSG_CHECK,
    /* Reserving some instr IDs for possible MSG ins additions */
    INS_GET_CLOSEST_ENM = 10,
    INS_ENM_DAMAGE,
    INS_ENM_DAMAGE_ITER,
    INS_ENM_DAMAGE_RADIUS,
    INS_ENM_DAMAGE_RECT,
    INS_ENM_ITERATOR,
    INS_ENM_ID_FROM_ITER,
    INS_ENM_ITER_FROM_ID,
    INS_ENM_FLAGS,
    INS_ENM_FLAGS_ITER,
    INS_ENM_HP,
    INS_ENM_HP_ITER,
    INS_ENM_POS_ITER
};

typedef struct {
    FLOAT a;
    FLOAT b;
    FLOAT c;
    FLOAT d;
} MESSAGE;

typedef std::list<MESSAGE*> MESSAGELIST;

#define GetEnmById ((ENEMYFULL* (__stdcall *)(DWORD))0x0041DDD0)

/* INS_2200 series: enemy communication */
BOOL ins_2200(ENEMY* enm, INSTR* ins);
