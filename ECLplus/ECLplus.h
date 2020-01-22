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

typedef LPVOID ENEMY;
typedef double DOUBLE;

#define EXPORT_LOC ((DWORD*)0x00499FE8)
#define GameGetIntArg 0x00428CC0
#define GameGetFloatArg 0x00428CF0
#define GameGetIntArgEx 0x00428D30
#define GameGetFloatArgEx 0x00428DE0

// #define GetVm(x) (DWORD*)(*((DWORD*)(*(x+0x000044D8))+0x0C))


/* Writes pointers to DLL functions to game's memory. */
VOID init();

/* Returns the nth ECL argument as an integer. */
DWORD GetIntArg(ENEMY enm, DWORD n);

/* Returns the nth ECL argument as an integer. Does not read the raw value
 * from the instruction, but from the given value. The game will read it as
 * variable if necessary, based on paramMask of the current instruction. */
DWORD GetIntArgEx(ENEMY enm, DWORD n, DWORD val);

/* Returns the nth ECL argument as a float. */
FLOAT GetFloatArg(ENEMY enm, DWORD n);

/* Returns the nth ECL argument as a float. Does not read the raw value
 * from the instruction, but from the given value. The game will read it as
 * variable if necessary, based on paramMask of the current instruction. */
FLOAT GetFloatArgEx(ENEMY enm, DWORD n, FLOAT val);

/* Returns a pointer to the ECL string argument
   which is the nth arg. Assumes that there are
   no other string arguments in the ins. */
const CHAR* GetStringArg(INSTR* ins, DWORD n);

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

/* Prints given string on the given coordinates within the game window. */
inline VOID EclPrintRender(FLOAT x, FLOAT y, CONST CHAR* format, DWORD len, CHAR* data) {
    POINTFLOAT p;
    p.x = x;
    p.y = y;
    POINTFLOAT* pp = &p;
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
        push pp
        mov eax, GamePrintRenderArg
        push [eax]
        mov eax, GamePrintRender
        call eax
        add esp, 12
        add esp, len
    }
}