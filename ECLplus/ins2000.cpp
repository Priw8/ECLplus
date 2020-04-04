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
#include "ins2000.h"
#include "ECLplus.h"

static DWORD EclArgList(CHAR* args, ENEMY* enm, INSTR* ins, PARAMD* D, DWORD i) {
    DWORD ind = 0;
    for (i; i < ins->paramCount; ++i) {
        DOUBLE* farg = (DOUBLE*)&args[ind];
        LONG* iarg = (LONG*)&args[ind];
        if (D->typeFrom == 'f') {
            FLOAT val = GetFloatArgEx(enm, i, D->val.f);
            if (D->typeTo == 'f') {
                *farg = val;
                ind += 8;
            }
            else if (D->typeTo == 'i') {
                *iarg = (LONG)val;
                ind += 4;
            }
        }
        else if (D->typeFrom == 'i') {
            LONG val = GetIntArgEx(enm, i, D->val.S);
            if (D->typeTo == 'f') {
                *farg = (DOUBLE)val;
                ind += 8;
            }
            else if (D->typeTo == 'i') {
                *iarg = val;
                ind += 4;
            }
        }
        ++D;
    }
    return ind;
}

static VOID EclPrintf(CHAR* buf, DWORD bufSize, INSTR* ins, ENEMY* enm) {
    CONST CHAR* str = GetStringArg(ins, 0);
    DWORD size = *(DWORD*)(&ins->data[0]); /* this includes padding (its purpose is keeping the rest of the params aligned) */
    PARAMD* D = (PARAMD*)&ins->data[size + 4]; /* +4 to include the size param */
    
    CHAR args[16*8];
    EclArgList(args, enm, ins, D, 1);

    vsnprintf(buf, bufSize, str, (va_list)args);
}

BOOL ins_2000(ENEMY* enm, INSTR* ins) {
    CHAR buf[512];
    switch (ins->id) {
    case INS_MSG_BOX:
        EclPrintf(buf, sizeof(buf), ins, enm);
        EclMsg(buf);
        break;
#ifdef DEV
    case INS_PRINTF:
        EclPrintf(buf, sizeof(buf), ins, enm);
        EclPrint(buf);
        break;
    case INS_CLS:
        system("cls");
        break;
#endif
    case INS_DRAW_TEXT: {
        FLOAT x = GetFloatArg(enm, 0);

        FLOAT y = GetFloatArg(enm, 1);

        DWORD size = *(DWORD*)(&ins->data[8]);
        CONST CHAR* format = GetStringArg(ins, 2);

        PARAMD* D = (PARAMD*)&ins->data[size + 12];
        DWORD len = EclArgList(buf, enm, ins, D, 3);

        EclPrintRender(x + 192.0f + 32.0f, y + 16.0f, format, len, buf);
        break;
    }
    case INS_TEXT_COLOR:
        SetDwordField(Deref(GamePrintRenderArg), GamePrintRenderStructColor, GetIntArg(enm, 0));
        break;
    case INS_BGM_SWITCH:
        /* Max 16 bytes because that's the max size of a name in thbgm.fmt file. */
        strcpy_s((char*)0x00528e88, 16, GetStringArg(ins, 0));
        /* Assembly code based on MSG ins_19, which starts boss BGM (th17.exe+2ea75). */
        __asm {
            push 0x004A1AE8
            push 1
            push 2
            mov ecx, 0x005247F0
            mov eax, 0x004662E0
            call eax
        }
        break;
    case INS_EXIT:
        GamemodeNext = 4; /* GAMEMODE_MENU */
        break;
    case INS_ITEM_SLOWDOWN:
        GameItemMgr->slowdown = GetFloatArg(enm, 0);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}
