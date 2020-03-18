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

enum INS2200 {
    INS_MSG_RESET = 2200,
    INS_MSG_RESET_CHANNEL,
    INS_MSG_SEND,
    INS_MSG_RECEIVE,
    INS_MSG_PEEK,
    INS_MSG_CHECK,
    /* Reserving some instr IDs for possible MSG ins additions */
    INS_GET_CLOSEST_ENM = 2210,
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
    INS_ENM_POS_ITER,
    INS_ENM_BOMBINVULN,
    INS_ENM_BOMBINVULN_ITER
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
