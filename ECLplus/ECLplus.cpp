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

#include "pch.h"
#include "ECLplus.h"
#include "ins2000.h"
#include "ins2100.h"
#include "ins2200.h"
#include "intvar.h"
#include "binhack.h"
#include "priority.h"

/* Executes an extra ECL instruction, called by the modified game. */
VOID __stdcall InsSwitch(ENEMY* enm, INSTR* ins) {
    BOOL success;
    if (ins->id >= 2000 && ins->id < 2100) {
        success = ins_2000(enm, ins);
    } else if (ins->id >= 2100 && ins->id < 2200) {
        success = ins_2100(enm, ins);
    } else if (ins->id >= 2200 && ins->id < 2300) {
        success = ins_2200(enm, ins);
    }
#ifdef DEV
    else {
        success = TRUE;
        EclMsgf("instruction out of range: %d", ins->id);
    }
    if (!success) {
        EclMsgf("bad instruction number: %d", ins->id);
    }
#endif
}

DWORD __stdcall IntVarSwitch(ENEMY* enm, DWORD var, DWORD type) {
    if (type == 0)
        return IntVarGetVal(enm, var);
    else
        return (DWORD)IntVarGetAddr(enm, var);
}

LONG DECLSPEC_NOINLINE GetIntArg(ENEMY* enm, DWORD n) {
    /* Calling the __thiscall requires assembly */
    LONG res;
    __asm {
        mov ecx, enm
        push n
        mov eax, GameGetIntArg
        call eax
        mov res, eax
    }
    return res;
}

LONG DECLSPEC_NOINLINE GetIntArgEx(ENEMY* enm, DWORD n, DWORD val) {
    LONG res;
    _asm {
        mov ecx, enm
        push val
        push n
        mov eax, GameGetIntArgEx
        call eax
        mov res, eax
    }
    return res;
}

DECLSPEC_NOINLINE FLOAT GetFloatArg(ENEMY* enm, DWORD n) {
    FLOAT res;
    __asm {
        mov ecx, enm
        push n
        mov eax, GameGetFloatArg
        call eax
        movd res, xmm0
    }
    return res;
}

DECLSPEC_NOINLINE FLOAT GetFloatArgEx(ENEMY* enm, DWORD n, FLOAT val) {
    FLOAT res;
    __asm {
        mov ecx, enm
        push n
        movss xmm2, val
        mov eax, GameGetFloatArgEx
        call eax
        movd res, xmm0
    }
    return res;
}

const CHAR* GetStringArg(INSTR* ins, DWORD n) {
    return (const CHAR*) &(ins->data[n * 4 + 4]);
}

DECLSPEC_NOINLINE LONG* GetIntArgAddr(ENEMY* enm, DWORD n) {
    LONG* res;
    __asm {
        mov ecx, enm
        push n
        mov eax, GameGetIntArgAddr
        call eax
        mov res, eax
    }
    return res;
}

DECLSPEC_NOINLINE FLOAT* GetFloatArgAddr(ENEMY* enm, DWORD n) {
    FLOAT* res;
    __asm {
        mov ecx, enm
        push n
        mov eax, GameGetFloatArgAddr
        call eax
        mov res, eax
    }
    return res;
}

VOID InitConsole() {
    AllocConsole();
    /* AllocConsole could've failed if another console was already
     * open (for example thcrap). Because of that, we don't show a message
     * on error, since the console might actually be there, fully functional.
     * Instead, the EclPrint function will display an error if it's
     * unable to get the stdout handle. */
    EclPrint("ECLplus v0.4 by Priw8\n");
}

VOID __stdcall MainLoop() {
    /* Called every frame. */
    /* Better to do priorities here rather than in init to ensure that the update func registry exists. */

    static DWORD hasAlreadyRun;
    if (!hasAlreadyRun) {
        InitPriorities();
        hasAlreadyRun = 1;
    }
}

VOID init() {
#ifdef DEV
    InitConsole();
#endif
    InitBinhacks();
}

