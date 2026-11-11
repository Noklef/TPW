#include "gameerror.h"

#include <cstdint>

#include "crt.h"
#include "fieldops.h"
#include "imports.h"
#include "staticobjects.h"

namespace {

AddrCalls g_calls;

// The string table's entry of each index (0xc and 0xd have none).
const u8 kEntries[0x2b] = {0x05, 0x03, 0x0b, 0x0c, 0x12, 0x14, 0x15, 0x16, 0x13, 0x17, 0x18, 0x19, 0, 0, 0x2a, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22,
                           0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x19, 0x01, 0x1a, 0x31, 0x32, 0x33, 0x1a};

}  // namespace

void GameError_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

void __fastcall GameError_Show(u8 *report) {
    u32 *const fields = reinterpret_cast<u32 *>(report);
    if (!fields[0] || !*reinterpret_cast<const u32 *>((uintptr_t)&gLanguage_78b530)) return;
    const u32 index = fields[1];
    if (index == 0xc) {
        u8 *const errors = reinterpret_cast<u8 *>((uintptr_t)0x00879ad0);
        u32 code = 0;
        while (const u8 *const entry = Entries8_Pop(errors))
            if (*reinterpret_cast<const u32 *>(entry) == 3) code = *reinterpret_cast<const u32 *>(entry + 4);
        fields[1] = GameError_IndexOfCode(report, nullptr, code);
        fields[2] = 0;
        fields[0] = 1;
        GameError_Show(report);
        return;
    }
    u16 text[0x100];   // the original's frame, 0x200 (left unset for an index without an entry, sic)
    if (index <= 0x2a && index != 0xd)
        g_calls.Cd(0x006acd60, *reinterpret_cast<const u32 *>((uintptr_t)&gLanguage_78b530 + 0x558), text, 0x100u, static_cast<u32>(kEntries[index]));
    if (report[0xc]) Crt().swprintf(text, reinterpret_cast<const u16 *>((uintptr_t)0x007477d0), text, report + 0xc);
    using MessageBox = s32(__stdcall *)(void *, const u16 *, const u16 *, u32);
    (*reinterpret_cast<const MessageBox *>((uintptr_t)IMPORT(User32_MessageBoxW)))(nullptr, text, reinterpret_cast<const u16 *>((uintptr_t)0x007477ac), 0x10010);
}


u32 __fastcall GameError_IndexOfCode(void *, void *, u32 code) {
    switch (code) {
    case 0xb0754100u: return 0x27;
    case 0xb0754101u: return 0x28;
    case 0xb0754102u: return 0x29;
    }
    const u32 offset = code - 0xb0754000u;  // (a jump table of 0x19 entries, each the offset + 0xe)
    return offset <= 0x18 ? offset + 0xe : 0x2a;
}

void __fastcall GameError_Set(u8 *report, void *, u32 index, u32 arg, const char *text) {
    *reinterpret_cast<u32 *>(report + 4) = index;
    *reinterpret_cast<u32 *>(report + 8) = arg;
    if (text) Crt().strncpy(reinterpret_cast<char *>(report + 0xc), text, 0x50);
    *reinterpret_cast<u32 *>(report) = 1;
}
}
