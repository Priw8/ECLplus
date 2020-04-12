#pragma once

#include "pch.h"

/*  
    *_ADDR - location where pointer to a DLL function is written
    *_CODECAVE - empty space where our code is written, often calls function in *_ADDR
    *_JUMP - where to overwrite part of game's code to jump to the codecave
*/

#define INS_SWITCH_ADDR ((LPVOID)0x00499FE8)
#define INS_SWITCH_CODECAVE ((LPVOID)0x00499EBA)
#define INS_SWITCH_JUMP ((LPVOID)0x004211AB)

#define INTVAR_SWITCH_ADDR ((LPVOID)0x00499FE4)
#define INTVAR_VAL_SWITCH_JUMP ((LPVOID)0x00427524)
#define INTVAR_VAL_SWITCH_CODECAVE ((LPVOID)0x00499ECA)
#define INTVAR_ADDR_SWITCH_JUMP ((LPVOID)0x00427Cf1)
#define INTVAR_ADDR_SWITCH_CODECAVE ((LPVOID)0x00499EDB)

#define ENMDMG_CODECAVE ((LPVOID)0x00499EEE)
#define ENMDMG_JUMP ((LPVOID)0x0041FA15)

#define PLAYER_DAMAGE_NOP ((LPVOID)0x0041E94F)
#define PLAYER_SPEED_NOP ((LPVOID)0x00448295)

#define MAINLOOP_ADDR ((LPVOID)0x00499FE0)
#define MAINLOOP_CODECAVE ((LPVOID)0x00499F0A)
#define MAINLOOP_JUMP ((LPVOID)0x004612DE)

typedef struct {
    LPVOID addr;
    CONST UCHAR* code;
    DWORD codelen;
} BINHACK;

extern CONST BINHACK binhacks[];
