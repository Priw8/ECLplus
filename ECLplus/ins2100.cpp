#include "pch.h"
#include "ins2100.h"
#include "ECLplus.h"

BOOL ins_2100(ENEMY enm, INSTR* ins) {
    switch(ins->id - 2100) {
        case INS_PLAYER_POS:
            SetDwordField(Player, PlayerXField, (LONG)(GetFloatArg(enm, 0) * 128.0f));
            SetDwordField(Player, PlayerYField, (LONG)(GetFloatArg(enm, 1) * 128.0f));
            break;
        case INS_PLAYER_KILL:
            SetDwordField(Player, PlayerStateField, 4);
            break;
        case INS_PLAYER_BOMB:
            ((void(__stdcall *)())0x00411C30)();
            break;
        case INS_PLAYER_SET_LIVES:
            PlayerLives = GetIntArg(enm, 0);
            // look at code around th17.exe+49286 for reference
            __asm {
                mov ecx, DWORD_PTR[0x004B76AC] // this ptr
                push DWORD_PTR[0x004B5A44]
                push DWORD_PTR[PlayerLivesPtr] 
                mov eax, 0x0042FC60
                call eax
            }
            break;
        case INS_PLAYER_SET_BOMBS:
            PlayerBombs = GetIntArg(enm, 0);
            // look at code around th17.exe+49286 for reference
            __asm {
                mov ecx, DWORD_PTR[0x004B76AC]
                push DWORD_PTR[0x004B5A50]
                push DWORD_PTR[PlayerBombsPtr]
                mov eax, 0x0042FD50
                call eax
            }
            break;
        default:
            return FALSE;
    }
    return TRUE;
}