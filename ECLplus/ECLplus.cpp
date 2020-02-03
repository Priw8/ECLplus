#include "pch.h"
#include "ECLplus.h"
#include "ins2000.h"
#include "ins2100.h"
#include "intvar.h"

/* Executes an extra ECL instruction, called by the modified game. */
static VOID __stdcall InsSwitch(ENEMY enm, INSTR* ins) {
    CHAR buf[256];
    BOOL success;
    if (ins->id >= 2000 && ins->id < 2100) {
        success = ins_2000(enm, ins);
    } else if (ins->id >= 2100 && ins->id < 2200) {
        success = ins_2100(enm, ins);
    } else {
        snprintf(buf, 256, "instruction out of range: %d", ins->id);
        EclMsg(buf);
    }
    if (!success) {
        snprintf(buf, 256, "bad instruction number: %d", ins->id);
        EclMsg(buf);
    }
}

static DWORD __stdcall IntVarSwitch(ENEMY enm, DWORD var, DWORD type) {
    if (type == 0)
        return IntVarGetVal(enm, var);
    else
        return (DWORD)IntVarGetAddr(enm, var);
}

DWORD GetIntArg(ENEMY enm, DWORD n) {
    /* Calling the __thiscall requires assembly */
    DWORD res;
    __asm {
        mov ecx, enm
        push n
        mov eax, GameGetIntArg
        call eax
        mov res, eax
    }
    return res;
}

DWORD GetIntArgEx(ENEMY enm, DWORD n, DWORD val) {
    DWORD res;
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

FLOAT GetFloatArg(ENEMY enm, DWORD n) {
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

FLOAT GetFloatArgEx(ENEMY enm, DWORD n, FLOAT val) {
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

VOID InitConsole() {
    AllocConsole();
    /* AllocConsole could've failed if another console was already
     * open (for example thcrap). Because of that, we don't show a message
     * on error, since the console might actually be there, fully functional.
     * Instead, the EclPrint function will display an error if it's
     * unable to get the stdout handle. */
    EclPrint("ECLplus v0.1 by Priw8\n");
}

/* TODO: rework how binhacks are applied (make some array of structures and iterate over it) */
static CONST UCHAR binhackInsCall[] = { 0x52, 0x57, 0xA1, 0xE8, 0x9F, 0x49, 0x00, 0xFF, 0xD0, 0xB8, 0xF2, 0x65, 0x42, 0x00, 0xFF, 0xE0 };
static CONST UCHAR binhackInsCallJump[] = { 0x0F, 0x87, 0x09, 0x8D, 0x07, 0x00 };

static CONST UCHAR binhackIntVar[] = { 0x6A, 0x00, 0x52, 0x50, 0xA1, 0xE4, 0x9F, 0x49, 0x00, 0xFF, 0xD0, 0x8B, 0xE5, 0x5D, 0xC2, 0x04, 0x00 };
static CONST UCHAR binhackIntVarJump[] = { 0x0F, 0x87, 0xA0, 0x29, 0x07, 0x00 };

static CONST UCHAR binhackIntVarAddr[] = { 0x6A, 0x01, 0x52, 0x83, 0xC0, 0x0F, 0x50, 0xA1, 0xE4, 0x9F, 0x49, 0x00, 0xFF, 0xD0, 0x5E, 0x5D, 0xC2, 0x04, 0x00 };
static CONST UCHAR binhackIntVarAddrJump[] = { 0x0F, 0x87, 0xE4, 0x21, 0x07, 0x00 };
VOID init() {
    InitConsole();
    DWORD old;
    /* Some code of the game has to be overwritten to call DLL functions. */
    /* ECL instructions: */
    VirtualProtect(EXPORT_LOC, 4, PAGE_EXECUTE_READWRITE, &old);
    *(LPVOID*)EXPORT_LOC = (LPVOID)InsSwitch;
    VirtualProtect(EXPORT_LOC, 4, old, &old);

    VirtualProtect(CODECAVE_LOC, sizeof(binhackInsCall), PAGE_READWRITE, &old);
    CopyMemory(CODECAVE_LOC, binhackInsCall, sizeof(binhackInsCall));
    VirtualProtect(CODECAVE_LOC, sizeof(binhackInsCall), old, &old);

    VirtualProtect(INS_HANDLER_LOC, sizeof(binhackInsCallJump), PAGE_READWRITE, &old);
    CopyMemory(INS_HANDLER_LOC, binhackInsCallJump, sizeof(binhackInsCallJump));
    VirtualProtect(INS_HANDLER_LOC, sizeof(binhackInsCallJump), old, &old);

    /* Integer variables: */
    VirtualProtect(EXPORT_INTVAR_LOC, 4, PAGE_EXECUTE_READWRITE, &old);
    *(LPVOID*)EXPORT_INTVAR_LOC = (LPVOID)IntVarSwitch;
    VirtualProtect(EXPORT_INTVAR_LOC, 4, old, &old);

    VirtualProtect(CODECAVE_INTVARGET_LOC, sizeof(binhackIntVar), PAGE_READWRITE, &old);
    CopyMemory(CODECAVE_INTVARGET_LOC, binhackIntVar, sizeof(binhackIntVar));
    VirtualProtect(CODECAVE_INTVARGET_LOC, sizeof(binhackIntVar), old, &old);

    VirtualProtect(INTVARGET_HANDLER_LOC, sizeof(binhackIntVarJump), PAGE_READWRITE, &old);
    CopyMemory(INTVARGET_HANDLER_LOC, binhackIntVarJump, sizeof(binhackIntVarJump));
    VirtualProtect(INTVARGET_HANDLER_LOC, sizeof(binhackIntVarJump), old, &old);

    VirtualProtect(CODECAVE_INTVARADDR_LOC, sizeof(binhackIntVarAddr), PAGE_READWRITE, &old);
    CopyMemory(CODECAVE_INTVARADDR_LOC, binhackIntVarAddr, sizeof(binhackIntVarAddr));
    VirtualProtect(CODECAVE_INTVARADDR_LOC, sizeof(binhackIntVarAddr), old, &old);

    VirtualProtect(INTVARADDR_HANDLER_LOC, sizeof(binhackIntVarAddrJump), PAGE_READWRITE, &old);
    CopyMemory(INTVARADDR_HANDLER_LOC, binhackIntVarAddrJump, sizeof(binhackIntVarAddrJump));
    VirtualProtect(INTVARADDR_HANDLER_LOC, sizeof(binhackIntVarAddrJump), old, &old);
}

