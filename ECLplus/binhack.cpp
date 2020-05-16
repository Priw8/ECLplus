#include "pch.h"
#include "ECLplus.h"
#include "binhack.h"
#include "priority.h"

static void ApplyPriorityBinhacks();

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
    CODE_WRITER& WriteNopTill(LPCVOID ptr) {
        return this->WriteNop((CONST UCHAR*)ptr - this->addr);
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

    {
        // Initialize extra enemy fields
        CODE_WRITER binhack = { (LPVOID)0x41e257 };

        CONST CHAR original[] = { '\x89', '\x86', '\x64', '\x57', '\x00', '\x00' };

        binhack.Expect(original);

        binhack.Write({ '\xE9' }); // jmp CAVE
        binhack.WriteRel(cave.addr);
        binhack.WriteNopTill((LPVOID)0x41e25d);

        cave.Write(original);                   // mov   dword [esi+0x5764], eax
        cave.Write({ '\xff', '\x75', '\x10' }); // push  dword [ebp + 0x10]
        cave.Write({ '\x56' });                 // push  esi
        cave.Write({ '\xE8' });
        cave.WriteRel(InitEnemyExFields);
        cave.Write({ '\xE9' });          // jmp  0x41e25d
        cave.WriteRel((LPVOID)0x41e25d);
        cave.Write({ '\xCC' });          // (unreachable)
        cave.Commit();
        binhack.Commit();
    }

    {
        // EnemyManager::on_tick runs the default rungroup
        CODE_WRITER binhack = { (LPVOID)0x41e8bc };

        // we've got 97 bytes of space here, no need for a codecave
        binhack.Write({ '\xE8' });  // call  RunEnemiesForEnemyManager
        binhack.WriteRel(RunEnemiesForEnemyManager);
        binhack.Write({ '\xE9' });  // jmp AFTER_LOOP
        binhack.WriteRel((LPVOID)0x41e91c);
        binhack.WriteNopTill((LPVOID)0x41e91c);
        binhack.Commit();
    }

    {
        // After an enemy runs its first tick, set its default priority.
        CODE_WRITER binhack = { (LPVOID)0x41e31b };

        CONST CHAR original[] = { '\x83', '\xbf', '\x80', '\x01', '\x00', '\x00', '\x00' };

        binhack.Expect(original);

        binhack.Write({ '\xE9' }); // jmp CAVE
        binhack.WriteRel(cave.addr);
        binhack.WriteNopTill((LPVOID)0x41e322);

        cave.Write({ '\x56' });          // push esi
        cave.Write({ '\xE8' });          // call
        cave.WriteRel(AfterNewEnemyRunsFirstTick);
        cave.Write(original);            // cmp  dword [edi + 0x180], 0x0
        cave.Write({ '\xE9' });          // jmp  AFTER
        cave.WriteRel((LPVOID)0x41e322);
        cave.Write({ '\xCC' });          // (unreachable)
        cave.Commit();
        binhack.Commit();
    }

    {
        // DEBUG DEBUG XXX
        CODE_WRITER binhack = { (LPVOID)0x42070c };

        CONST CHAR original[] = { '\xf3', '\x0f', '\x10', '\x93', '\xf0', '\x14', '\x00', '\x00' };

        binhack.Expect(original);

        binhack.Write({ '\xE9' }); // jmp CAVE
        binhack.WriteRel(cave.addr);
        binhack.WriteNopTill((LPVOID)0x420714);

        cave.Write({ '\x51' }); // push ecx
        cave.Write({ '\xE8' }); // call DebugStuffStuff
        cave.WriteRel(DebugStuffStuff);
        cave.Write(original);
        cave.Write({ '\xE9' }); // jmp
        cave.WriteRel((LPVOID)0x420714);
        cave.Write({ '\xCC' }); // (unreachable)
        cave.Commit();
        binhack.Commit();
    }

    {
        // callsite of 0x41480d that creates a child
        CODE_WRITER binhack = { (LPVOID)0x426d79 };

        binhack.Write({ '\xE9' }); // jmp CAVE
        binhack.WriteRel(cave.addr);
        binhack.WriteNopTill((LPVOID)0x426d7e);

        cave.Write({ '\x56' }); // push esi
        cave.Write({ '\xE8' }); // call
        cave.WriteRel(PatchedCreateChildEnemy);
        cave.Write({ '\xE9' }); // jmp
        cave.WriteRel((LPVOID)0x426d7e);
        cave.Write({ '\xCC' }); // (unreachable)
        cave.Commit();
        binhack.Commit();

        // callsites that aren't creating children
        std::array<DWORD, 3> addrs = { 0x417ad4, 0x431c58, 0x431e83 };
        for (auto it = addrs.begin(); it != addrs.end(); it++) {
            binhack = { (LPVOID)*it };

            binhack.Write({ '\xE8' });
            binhack.WriteRel(PatchedCreateNonChildEnemy);
            binhack.Commit();
        }
    }

    // Player damage multiplier nop
    CODE_WRITER { (LPVOID)0x0041E94F }.WriteNop(10).Commit();

    ApplyPriorityBinhacks();

    // (note: not giving this write permission crashes for some reason...?)
    DWORD old;
    VirtualProtect(cavesStart, CAVES_MAXSIZE, PAGE_EXECUTE_READWRITE, &old);

    // Copy func pointers to a location in static memory where downstream patches can find them.
    VirtualProtect(FUNC_POINTERS_BEGIN, sizeof(funcs), PAGE_EXECUTE_READWRITE, &old);
    *FUNC_POINTERS_BEGIN = funcs;
    VirtualProtect(FUNC_POINTERS_BEGIN, sizeof(funcs), old, &old);
}

static void ApplyPriorityBinhacks() {
    struct PRIORITY_BINHACK {
        DWORD addr;
        DWORD oldPriority;
    };

    // !!!!!!!!!!!!!!!!!!
    // TODO: Fix priority of functions already registered.
    // Should do both that and this whlie holding the CRITICAL_SECTION for UpdateFuncManager.
    // !!!!!!!!!!!!!!!!!!

    // convert all 44 existing UpdateFunc priorities to use
    PRIORITY_BINHACK binhacks[] = {
        { 0x407dae, 0x05 }, // Ascii
        { 0x409c21, 0x12 }, // Stage
        { 0x40a7b3, 0x14 }, // Screen effect from Stage
        { 0x40e4ef, 0x1f }, // Tokens
        { 0x411686, 0x19 }, // ShotType
        { 0x411ecd, 0x14 }, // Marisa bomb screen shake
        { 0x4131a6, 0x14 }, // Reimu bomb screen effects 1
        { 0x4138c5, 0x14 }, // Reimu bomb screen effects 2
        { 0x413b7e, 0x14 }, // Reimu bomb screen effects 3
        { 0x413e58, 0x14 }, // Youmu bomb screen shake
        { 0x4148a8, 0x1d }, // Bullets
        { 0x41befc, 0x25 }, // Ending????
        { 0x41d0b8, 0x14 }, // Some screen effect (in Ending????)
        { 0x41d175, 0x14 }, // Some screen effect (in Ending????)
        { 0x41e61f, 0x1b }, // Enemies
        { 0x42b53f, 0x22 }, // GUI/HUD
        { 0x4305ac, 0x10 }, // Game thread (Cutoff for pause menu)
        { 0x43074a, 0x1c }, // Lasers
        { 0x430840, 0x0b }, // Pause & Game over menu
        { 0x430ae2, 0x20 }, // Enemy Spellcard
        { 0x4313e6, 0x14 }, // Screen effect from game thread
        { 0x431a9b, 0x14 }, // Screen effect from... who cares?
        { 0x43290e, 0x0c }, // Help manual
        { 0x43323c, 0x1e }, // Items
        { 0x440684, 0x21 }, // Global2020
        { 0x440b30, 0x04 }, // Global640
        { 0x441199, 0x02 }, // Global38
        { 0x441758, 0x01 }, // IO
        { 0x4464fa, 0x17 }, // Player
        { 0x44de45, 0x11 }, // Game thread again (Game input)
        { 0x44deba, 0x24 }, // Game thread again (???)
        { 0x44e05f, 0x11 }, // Game thread again (Game input)
        { 0x44e07d, 0x24 }, // Game thread again (???)
        { 0x45045d, 0x15 }, // Kanji number popups
        { 0x4510e4, 0x07 }, // Main menu
        { 0x45da0d, 0x0e }, // Trophies
        { 0x464967, 0x14 }, // More screen effects
        { 0x464996, 0x14 }, // ...
        { 0x4649d0, 0x14 }, // ...
        { 0x4649f9, 0x14 }, // ...
        { 0x464a1b, 0x14 }, // ...
        { 0x464a3b, 0x14 }, // ...
        { 0x471d5d, 0x23 }, // World ANMs
        { 0x471dc5, 0x0a }, // UI ANMs
    };
    SIZE_T n = sizeof(binhacks) / sizeof(PRIORITY_BINHACK);
    for (int i = 0; i < n; i++) {
        CODE_WRITER binhack = { (LPVOID)binhacks[i].addr };
        DWORD oldPriority = binhacks[i].oldPriority;
        DWORD newPriority = RUNGROUP { oldPriority, REL_DURING }.EffectivePriority();

        binhack.Expect({ '\x6a', (CHAR)oldPriority });
        binhack.Write({ '\x6a', (CHAR)newPriority });
        binhack.Commit();
    }
}
