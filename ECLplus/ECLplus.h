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

/* Borrowed from thecl */
#pragma pack(push, 1)
#pragma warning(push)
#pragma warning(disable: 4200)
typedef struct {
    DWORD time;
    WORD id;
    WORD size;
    WORD paramMask;
    /* The rank bitmask.
     *   1111LHNE
     * Bits mean: easy, normal, hard, lunatic. The rest are always set to 1. */
    BYTE rankMask;
    /* There doesn't seem to be a way of telling how many parameters there are
     * from the additional data. */
    BYTE paramCount;
    /* From TH13 on, this field stores the number of current stack references
     * in the parameter list. */
    DWORD zero;
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

typedef struct {
    LPVOID addr;
    CONST UCHAR* code;
    DWORD codelen;
} BINHACK;

#pragma pack(push, 1)
typedef struct ENEMY {
    CHAR pad0[0x44];
    POINTFLOAT pos;

    #define lpadHurtbox (sizeof(pad0) + sizeof(pos))
    CHAR padHurtbox[0x110 - lpadHurtbox];
    POINTFLOAT hurtbox;

    #define lpad1 (lpadHurtbox + sizeof(padHurtbox) + sizeof(hurtbox))
    CHAR pad1[0x3F54 - lpad1];
    LONG pendingDmg;

    #define lpad2 (lpad1 + sizeof(pad1) + sizeof(pendingDmg))
    CHAR pad2[0x3F74 - lpad2];
    LONG hp;
    LONG hpMax;
    LONG hpTreshold;

    #define lpad3 (lpad2 + sizeof(pad2) + sizeof(hp) + sizeof(hpMax) + sizeof(hpTreshold))
    CHAR pad3[0x4080 - lpad3];
    DWORD flags;

    #define lpad4 (lpad3 + sizeof(pad3) + sizeof(flags))
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

typedef double DOUBLE;

#define SetDwordField(ptr,off,val) (*((DWORD*)(ptr + off)) = val)
#define SetFloatField(ptr,off,val) (*((FLOAT*)(ptr + off)) = val)
#define GetDwordField(ptr,off) (*((DWORD*)(ptr + off)))
#define GetFloatField(ptr,off) (*((FLOAT*)(ptr + off)))
#define Deref(x) (*(DWORD*)x)

#define GameEnmMgr ((ENEMYMGR*)Deref(0x004B76A0))

#define EXPORT_LOC ((LPVOID)0x00499FE8)
#define CODECAVE_LOC ((LPVOID)0x00499EBA)
#define INS_HANDLER_LOC ((LPVOID)0x004211AB)

#define EXPORT_INTVAR_LOC ((LPVOID)0x00499FE4)
#define INTVARGET_HANDLER_LOC ((LPVOID)0x00427524)
#define CODECAVE_INTVARGET_LOC ((LPVOID)0x00499ECA)
#define INTVARADDR_HANDLER_LOC ((LPVOID)0x00427Cf1)
#define CODECAVE_INTVARADDR_LOC ((LPVOID)0x00499EDB)

#define CODECAVE_ENMDMG_LOC ((LPVOID)0x00499EEE)
#define CODECAVE_ENMDMG_JUMP_LOC ((LPVOID)0x0041FA15)

#define GameGetIntArg 0x00428CC0
#define GameGetFloatArg 0x00428CF0
#define GameGetIntArgEx 0x00428D30
#define GameGetFloatArgEx 0x00428DE0

#define GameGetIntArgAddr 0x00428CE0
#define GameGetFloatArgAddr 0x00428D10

#define WriteIfNotNull(x, y) (if (x != NULL) *x = y)

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
    MessageBoxA(NULL, str, "ExtraIns", MB_OK);
}

/* Prints given string in the console. */
inline VOID EclPrint(CONST CHAR* str) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE || handle == NULL) {
        EclMsg("Unable to get stdout handle.");
        return;
    }
    DWORD wr;
    WriteConsoleA(handle, str, strlen(str), &wr, NULL);
}

#define GamePrintRender 0x004082B0
#define GamePrintRenderArg 0x004B7678
#define GamePrintRenderStructColor 0x00019214

/* Prints given string on the given coordinates within the game window. */
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
