#include "pch.h"
#include "ins2000.h"
#include "ECLplus.h"

static DWORD EclArgList(CHAR* args, ENEMY enm, INSTR* ins, PARAMD* D, DWORD i) {
    DWORD ind = 0;
    for (i; i < ins->paramCount; ++i) {
        DOUBLE* farg = (DOUBLE*)&args[ind];
        DWORD* iarg = (DWORD*)&args[ind];
        if (D->typeFrom == 'f') {
            FLOAT val = GetFloatArgEx(enm, i, D->val.f);
            if (D->typeTo == 'f') {
                *farg = val;
                ind += 8;
            }
            else if (D->typeTo == 'i') {
                *iarg = (DWORD)val;
                ind += 4;
            }
        }
        else if (D->typeFrom == 'i') {
            DWORD val = GetIntArgEx(enm, i, D->val.S);
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

static VOID EclPrintf(CHAR* buf, DWORD bufSize, INSTR* ins, ENEMY enm) {
    CONST CHAR* str = GetStringArg(ins, 0);
    DWORD size = *(DWORD*)(&ins->data[0]); /* this includes padding (its purpose is keeping the rest of the params aligned) */
    PARAMD* D = (PARAMD*)&ins->data[size + 4]; /* +4 to include the size param */
    
    CHAR args[16*8];
    EclArgList(args, enm, ins, D, 1);

    vsnprintf(buf, bufSize, str, (va_list)args);
}

BOOL ins_2000(ENEMY enm, INSTR* ins) {
    CHAR buf[512];
    switch (ins->id - 2000) {
    case INS_MSG_BOX:
        EclPrintf(buf, sizeof(buf), ins, enm);
        EclMsg(buf);
        break;
    case INS_PRINTF:
        EclPrintf(buf, sizeof(buf), ins, enm);
        EclPrint(buf);
        break;
    case INS_CLS:
        system("cls");
        break;
    case INS_DRAW_TEXT: {
        DWORD size = *(DWORD*)(&ins->data[0]);
        CONST CHAR* format = GetStringArg(ins, 0);

        FLOAT x = *(FLOAT*)(&ins->data[size + 4]);
        x = GetFloatArgEx(enm, 1, x);

        FLOAT y = *(FLOAT*)(&ins->data[size + 8]);
        y = GetFloatArgEx(enm, 2, y);

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
    default:
        return FALSE;
    }
    return TRUE;
}
