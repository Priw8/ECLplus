#include "pch.h"
#include "ECLplus.h"
#include "binhack.h"

struct CODE_WRITER {
    UCHAR* addr;
    std::vector<UCHAR> pending;

    CODE_WRITER(LPVOID ptr) : addr((UCHAR*)ptr), pending() {}

    // Perform all pending writes.
    // The purpose of this is to make it easier to ensure that codecaves are written before
    // the code that jumps to them.
    VOID Commit() {
        DWORD old;
        SIZE_T size = this->pending.size();
        UCHAR* start = this->addr - size;

        VirtualProtect(start, size, PAGE_EXECUTE_READWRITE, &old);
        CopyMemory(start, this->pending.data(), size);
        VirtualProtect(start, size, old, &old);

        this->pending.clear();
    }

    // Check the bytes at the current write cursor.
    template<SIZE_T SIZE>
    BOOL Expect(CONST CHAR (&arr)[SIZE]) {
        if (memcmp(this->addr, arr, SIZE)) {
            char buf[256];
            snprintf(buf, 256, "ECLplus binhack warning: Expected bytes not found at %#08x.", (DWORD)this->addr);
            EclMsg(buf);
            return FALSE;
        }
        return TRUE;
    }

    // Write bytes and advance the write cursor.
    // They will not actually be written to memory until Commit() is called.
    template<SIZE_T SIZE>
    CODE_WRITER& Write(CONST CHAR (&arr)[SIZE]) {
        this->pending.insert(this->pending.end(), arr, arr + SIZE);
        this->addr += SIZE;
        return *this;
    }

    CODE_WRITER& WriteDword(DWORD value) {
        CHAR littleEndianBytes[4];
        *(DWORD*)littleEndianBytes = value;
        return this->Write(littleEndianBytes);
    }
    CODE_WRITER& WriteAbs(LPCVOID ptr) {
        return this->WriteDword((DWORD)ptr);
    }
    CODE_WRITER& WriteRel(LPCVOID ptr) {
        return this->WriteDword((CONST UCHAR*)ptr - (this->addr + 4));
    }
    CODE_WRITER& WriteNop(SIZE_T size) {
        this->pending.resize(this->pending.size() + size, 0x90);
        this->addr += size;
        return *this;
    }
};

constexpr SIZE_T CAVES_MAXSIZE = 0x3000;
static UCHAR* cavesStart;

void InitBinhacks() {
    ECLPLUS_FUNCS funcs = {};
    cavesStart = new UCHAR[CAVES_MAXSIZE]();

    CODE_WRITER cave { cavesStart };

    {
        // ECL instructions
        CODE_WRITER binhack = { (LPVOID)0x4211AB };

        binhack.Write({ '\x0F', '\x87' }); // ja CAVE
        binhack.WriteRel(cave.addr);

        cave.Write({ '\x52' }); // push edx
        cave.Write({ '\x57' }); // push edi
        cave.Write({ '\xE8' }); // call InsSwitch
        cave.WriteRel(funcs.insSwitch);
        cave.Write({ '\xe9' }); // jmp  FUNC_END
        cave.WriteRel((LPVOID)0x4265f2);
        cave.Write({ '\xCC' }); // (unreachable)
        cave.Commit();
        binhack.Commit();
    }

    {
        // Int variables (by value)
        CODE_WRITER binhack = { (LPVOID)0x427524 };

        binhack.Write({ '\x0F', '\x87' }); // ja CAVE
        binhack.WriteRel(cave.addr);

        cave.Write({ '\x6a', '\x00' });         // push 0x0
        cave.Write({ '\x52' });                 // push edx
        cave.Write({ '\x50' });                 // push eax
        cave.Write({ '\xE8' });                 // call IntVarSwitch
        cave.WriteRel(funcs.intVarSwitch);
        cave.Write({ '\x8b', '\xe5' });         // mov esp, ebp
        cave.Write({ '\x5d' });                 // pop ebp
        cave.Write({ '\xc2', '\x04', '\x00' }); // ret 0x4
        cave.Write({ '\xCC' });                 // (unreachable)
        cave.Commit();
        binhack.Commit();
    }

    {
        // Int variables (by pointer)
        CODE_WRITER binhack = { (LPVOID)0x00427Cf1 };

        binhack.Write({ '\x0F', '\x87' }); // ja CAVE
        binhack.WriteRel(cave.addr);

        cave.Write({ '\x6a', '\x01' });         // push 0x1
        cave.Write({ '\x52' });                 // push edx
        cave.Write({ '\x83', '\xc0', '\x0f' }); // add eax, 0xf
        cave.Write({ '\x50' });                 // push eax
        cave.Write({ '\xE8' });                 // call IntVarSwitch
        cave.WriteRel(funcs.intVarSwitch);
        cave.Write({ '\x5e' });                 // pop esi
        cave.Write({ '\x5d' });                 // pop ebp
        cave.Write({ '\xc2', '\x04', '\x00' }); // ret 0x4
        cave.Write({ '\xCC' });                 // (unreachable)
        cave.Commit();
        binhack.Commit();
    }

    {
        // Enemy damage
        CODE_WRITER binhack = { (LPVOID)0x0041FA15 };

        binhack.Expect({ '\xe8', '\x76', '\xb6', '\x02', '\x00' });

        binhack.Write({ '\xE9' }); // jmp CAVE
        binhack.WriteRel(cave.addr);

        cave.Write({ '\xe8' }); // call TALLY_DAMAGE_FROM_PLAYER
        cave.WriteRel((LPCVOID)0x44b090);
        cave.Write({ '\x03', '\x83' });         // add  eax, DWORD PTR[ebx + pendingDmg]
        cave.WriteDword(GetExFieldOffset(pendingDmg));
        cave.Write({ '\xc7', '\x83' });         // mov  DWORD PTR[ebx + pendingDmg], 0x0
        cave.WriteDword(GetExFieldOffset(pendingDmg));
        cave.WriteDword(0);
        cave.Write({ '\xE9' });                 // jmp  0x41fa1a
        cave.WriteRel((LPVOID)0x41fa1a);
        cave.Write({ '\xCC' });                 // (unreachable)
        cave.Commit();
        binhack.Commit();
    }

    {
        // Main loop
        CODE_WRITER binhack = { (LPVOID)0x4612DE };

        CONST CHAR original[] = { '\x89', '\x8d', '\x8c', '\xdd', '\xff', '\xff' };

        binhack.Expect(original);

        binhack.Write({ '\xE9' }); // jmp CAVE
        binhack.WriteRel(cave.addr);
        binhack.WriteNop(1);

        cave.Write(original);            // mov  [ebp-0x2274], ecx
        cave.Write({ '\x60' });          // pusha
        cave.Write({ '\xe8' });          // call MainLoop
        cave.WriteRel(funcs.mainLoop);
        cave.Write({ '\x61' });          // popa
        cave.Write({ '\xE9' });          // jmp  0x4612e4
        cave.WriteRel((LPVOID)0x4612e4);
        cave.Write({ '\xCC' });          // (unreachable)
        cave.Commit();
        binhack.Commit();
    }

    // Player damage multiplier nop
    CODE_WRITER { (LPVOID)0x0041E94F }.WriteNop(10).Commit();

    // (note: not giving this write permission crashes for some reason...?)
    DWORD old;
    VirtualProtect(cavesStart, CAVES_MAXSIZE, PAGE_EXECUTE_READWRITE, &old);

    // Copy func pointers to a location in static memory where downstream patches can find them.
    VirtualProtect(FUNC_POINTERS_BEGIN, sizeof(funcs), PAGE_EXECUTE_READWRITE, &old);
    *FUNC_POINTERS_BEGIN = funcs;
    VirtualProtect(FUNC_POINTERS_BEGIN, sizeof(funcs), old, &old);
}
