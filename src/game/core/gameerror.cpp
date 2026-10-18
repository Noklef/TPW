#include "gameerror.h"

#include <cstdint>

#include "crt.h"

extern "C" {

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
