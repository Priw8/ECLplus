#pragma once

#include "pch.h"
#include "ECLplus.h"

#define FUNC_POINTERS_BEGIN ((ECLPLUS_FUNCS*)0x00499FE0)

#pragma pack(push, 1)
struct ECLPLUS_FUNCS {
    VOID(__stdcall* mainLoop)();
    DWORD(__stdcall* intVarSwitch)(ENEMY* enm, DWORD var, DWORD type);
    VOID(__stdcall* insSwitch)(ENEMY* enm, INSTR* ins);

    ECLPLUS_FUNCS()
        : mainLoop(MainLoop)
        , intVarSwitch(IntVarSwitch)
        , insSwitch(InsSwitch)
    {}
};
#pragma pack(pop)

void InitBinhacks();