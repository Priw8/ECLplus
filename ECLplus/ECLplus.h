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

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the ECLPLUS_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// ECLPLUS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#pragma once
#ifdef ECLPLUS_EXPORTS
#define ECLPLUS_API __declspec(dllexport)
#else
#define ECLPLUS_API __declspec(dllimport)
#endif

/* Adapted from thecl */
#pragma pack(push, 1)
#pragma warning(push)
#pragma warning(disable: 4200)
typedef struct {
    DWORD time;
    WORD id;
    WORD size;
    WORD paramMask;
    /* The rank bitmask.
     *   76OXLHNE
     * Bits mean: easy, normal, hard, lunatic, extra, overdrive and 2 unused difficulties (6 and 7) */
    BYTE rankMask;
    /* There doesn't seem to be a way of telling how many parameters there are
     * from the additional data. */
    BYTE paramCount;
    /* How many bytes the ECL stack pointer should be decreased by after executing the instruction. */
    DWORD popCnt;
    UCHAR data[];
} INSTR;
#pragma warning(pop)
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    union {
        DWORD S;
        FLOAT f;
    };
} MIXEDVAL;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    CHAR typeFrom;
    CHAR typeTo;
    CHAR padding[2];
    MIXEDVAL val;
} PARAMD;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct ENEMY {
    CHAR pad0[0x44];
    POINTFLOAT pos;

    #define lpadHurtbox (sizeof(pad0) + sizeof(pos))
    CHAR padHurtbox[0x110 - lpadHurtbox];
    POINTFLOAT hurtbox;
    POINTFLOAT hitbox;
    FLOAT rotation;

    #define lpad1 (lpadHurtbox + sizeof(padHurtbox) + sizeof(hurtbox) + sizeof(hitbox) + sizeof(rotation))
    CHAR pad1[0x3F54 - lpad1];
    LONG pendingDmg;

    #define lpad2 (lpad1 + sizeof(pad1) + sizeof(pendingDmg))
    CHAR pad2[0x3F74 - lpad2];
    LONG hp;
    LONG hpMax;
    LONG hpTreshold;

    #define lpad3 (lpad2 + sizeof(pad2) + sizeof(hp) + sizeof(hpMax) + sizeof(hpTreshold))
    CHAR pad3[0x407C - lpad3];
    FLOAT bombInvuln;
    DWORD flags;

    #define lpad4 (lpad3 + sizeof(pad3) + sizeof(flags) + sizeof(bombInvuln))
    CHAR pad4[0x4554 - lpad4];
    DWORD id;
} ENEMY;
#pragma pack(pop)

/* I think there is some baseObj class that enemy class is derived from. ENEMYFULL represents the full
 * enemy class including the baseObj, while ENEMY is just the enemy without the base class.  */
#pragma pack(push, 1)
typedef struct ENEMYFULL {
    CHAR pad[0x120C];
    ENEMY enm;
} ENEMYFULL;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct ENEMYLISTNODE {
    ENEMYFULL* obj;
    ENEMYLISTNODE* next;
} ENEMYLISTNODE;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct ENEMYMGR {
    CHAR pad1[0x180];
    ENEMYLISTNODE* head;
} ENEMYMGR;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct ITEMMGR {
    CHAR pad1[0xe4b974];
    FLOAT slowdown;
} ITEMMGR;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct SPELLCARD {
    CHAR pad1[0x78];
    DWORD flags;
    DWORD bonus;
    DWORD bonusMax;
} SPELLCARD;
#pragma pack(pop, 1)
#define SPELL_FLAG_CAPTURE 0x02

typedef double DOUBLE;

#define SetDwordField(ptr,off,val) (*((DWORD*)(ptr + off)) = val)
#define SetFloatField(ptr,off,val) (*((FLOAT*)(ptr + off)) = val)
#define GetDwordField(ptr,off) (*((DWORD*)(ptr + off)))
#define GetFloatField(ptr,off) (*((FLOAT*)(ptr + off)))
#define Deref(x) (*(DWORD*)x)

#define GameEnmMgr ((ENEMYMGR*)Deref(0x004B76A0))
#define GameItemMgr ((ITEMMGR*)Deref(0x004B76B8))
#define GameSpell ((SPELLCARD*)Deref(0x004B7690))

#define GameGetIntArg 0x00428CC0
#define GameGetFloatArg 0x00428CF0
#define GameGetIntArgEx 0x00428D30
#define GameGetFloatArgEx 0x00428DE0

#define GameGetIntArgAddr 0x00428CE0
#define GameGetFloatArgAddr 0x00428D10

#define WriteIfNotNull(x, y) (if (x != NULL) *x = y)

#define Gamemode *(DWORD*)(0x004B61D0)
#define GamemodeNext *(DWORD*)(0x004B61D4)

// #define GetVm(x) (DWORD*)(*((DWORD*)(*(x+0x000044D8))+0x0C))


/* Writes pointers to DLL functions to game's memory. */
VOID init();

/* Returns the nth ECL argument as an integer. */
LONG GetIntArg(ENEMY* enm, DWORD n);

/* Returns the nth ECL argument as an integer. Does not read the raw value
 * from the instruction, but from the given value. The game will read it as
 * variable if necessary, based on paramMask of the current instruction. */
LONG GetIntArgEx(ENEMY* enm, DWORD n, DWORD val);

/* Returns the nth ECL argument as a float. */
FLOAT GetFloatArg(ENEMY* enm, DWORD n);

/* Returns the nth ECL argument as a float. Does not read the raw value
 * from the instruction, but from the given value. The game will read it as
 * variable if necessary, based on paramMask of the current instruction. */
FLOAT GetFloatArgEx(ENEMY* enm, DWORD n, FLOAT val);

/* Returns a pointer to the ECL string argument
   which is the nth arg. Assumes that there are
   no other string arguments in the ins. */
const CHAR* GetStringArg(INSTR* ins, DWORD n);

/* Returns a pointer to the int variable passed as the nth argument.
 * NOTE: only works for n=0 because of optimizations MSVC did on WBaWC 
 * (game only ever calls this with n=0) */
LONG* GetIntArgAddr(ENEMY* enm, DWORD n);

/* Returns a pointer to the float variable passed as the nth argument. */
FLOAT* GetFloatArgAddr(ENEMY* enm, DWORD n);

/* Shows a message box of the given content, */
inline VOID EclMsg(CONST CHAR* str) {
    MessageBoxA(NULL, str, "ECLplus", MB_OK);
}

/* Prints given string in the console. */
#ifdef DEV
inline VOID EclPrint(CONST CHAR* str) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE || handle == NULL) {
        EclMsg("Unable to get stdout handle.");
        return;
    }
    DWORD wr;
    WriteConsoleA(handle, str, strlen(str), &wr, NULL);
}
#else
#define EclPrint(x)
#endif

#define GamePrintRender 0x004082B0
#define GamePrintRenderArg 0x004B7678
#define GamePrintRenderStructColor 0x00019214
#define GamePrintRenderStructFont 0x0001922c
#define GamePrintRenderStructAnchorX 0x00019238
#define GamePrintRenderStructAnchorY 0x0001923c
#define GamePrintRenderStructShadow 0x00019228

/* Prints given string on the given coordinates within the game window. 
 * 'data' contains the raw values that will be pushed to the stack as the
 * format data for the game's function (must be DWORD aligned length) */
inline VOID EclPrintRender(FLOAT x, FLOAT y, CONST CHAR* format, DWORD len, CHAR* data) {
    POINTFLOAT p;
    p.x = x;
    p.y = y;
    __asm {
        mov eax, data
        mov edx, eax
        add eax, len
    PUSH_LOOP:
        cmp eax, edx
        je PUSH_LOOP_END
        sub eax, 4
        push [eax]
        jmp PUSH_LOOP
    PUSH_LOOP_END:
        push format
        lea eax, p
        push eax
        mov eax, GamePrintRenderArg
        push [eax]
        mov eax, GamePrintRender
        call eax
        add esp, 12
        add esp, len
    }
}

/* Enemy flags */
#define FLAG_NO_HURTBOX 1
#define FLAG_NO_HITBOX 2
#define FLAG_OFFSCREEN_LR 4
#define FLAG_OFFSCREEN_UD 8
#define FLAG_INVINCIBLE 16
#define FLAG_INTANGIBLE 32
/* flag 64 is unknown */
#define FLAG_NO_DELETE 128
#define FLAG_ALWAYS_DELETE 256
#define FLAG_GRAZE 512
#define FLAG_ONLY_DIALOG_DELETE 1024
#define FLAG_ETCLEAR_DIE 2048
#define FLAG_RECT_HITBOX 4096

#define FLAG_BOMBSHIELD 268435456
