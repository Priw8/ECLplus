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
        default:
            return FALSE;
    }
    return TRUE;
}