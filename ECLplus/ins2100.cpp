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
#include "ins2100.h"
#include "ECLplus.h"

BOOL ins_2100(ENEMY* enm, INSTR* ins) {
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
        case INS_PLAYER_SET_POWER:
            PlayerPower = GetIntArg(enm, 0);
            __asm {
                mov ecx, DWORD_PTR[PlayerPtr]
                lea ecx, DWORD_PTR[ecx + 0x00000610]
                mov eax, 0x00449630
                call eax
            }
            break;
        case INS_PLAYER_SET_IFRAMES: {
            LONG iframes = GetIntArg(enm, 0);
            SetDwordField(Player, PlayerIframeIntField, iframes);
            SetFloatField(Player, PlayerIframeFloatField, (float)iframes);
            break;
        }
        case INS_PLAYER_TOGGLE_SHOT: {
            DWORD old;
            VirtualProtect((LPVOID)0x0044ABF5, 1, PAGE_READWRITE, &old);
            *(BYTE*)0x0044ABF5 = GetIntArg(enm, 0) ? 1 : 0;
            VirtualProtect((LPVOID)0x0044ABF5, 1, old, &old);

            VirtualProtect((LPVOID)0x0044AC4D, 1, PAGE_READWRITE, &old);
            *(BYTE*)0x0044AC4D = GetIntArg(enm, 0) ? 1 : 0;
            VirtualProtect((LPVOID)0x0044AC4D, 1, old, &old);

            VirtualProtect((LPVOID)0x0044ACAC, 1, PAGE_READWRITE, &old);
            *(BYTE*)0x0044ACAC = GetIntArg(enm, 0) ? 1 : 0;
            VirtualProtect((LPVOID)0x0044ACAC, 1, old, &old);
            break;
        }
        case INS_PLAYER_TOGGLE_BOMB: {
            DWORD old;
            VirtualProtect((LPVOID)0x00447B3E, 1, PAGE_READWRITE, &old);
            *(BYTE*)0x00447B3E = GetIntArg(enm, 0) ? 0x00 : 0x7F;
            VirtualProtect((LPVOID)0x00447B3E, 1, old, &old);

            VirtualProtect((LPVOID)0x00447C2C, 1, PAGE_READWRITE, &old);
            *(BYTE*)0x00447C2C = GetIntArg(enm, 0) ? 0x00 : 0x7F;
            VirtualProtect((LPVOID)0x00447C2C, 1, old, &old);
            break;
        }
        case INS_PLAYER_SET_HYPER_TIMER:
            PlayerHyperTimer = (FLOAT)GetIntArg(enm, 0);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}