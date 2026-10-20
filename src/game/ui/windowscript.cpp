#include "windowscript.h"

#include <cstdint>

namespace {

u32 &cursor() { return *reinterpret_cast<u32 *>((uintptr_t)0x00faa620); }

u16 next(u8 *script) {
    const u16 word = *reinterpret_cast<u16 *>(script + 2 * cursor());
    ++cursor();
    return word;
}
}  // namespace

extern "C" {

u32 __cdecl WindowScript_ReadDword(u8 *script) {
    u8 *const at = script + 2 * cursor();
    cursor() += 2;
    return *reinterpret_cast<u32 *>(at);
}

u32 __cdecl WindowScript_ReadWord(u8 *script) {
    const u32 old = cursor();
    return (old & 0xffff0000u) | next(script);
}

u16 *__cdecl WindowScript_ReadRect(u8 *script, u16 *a, u16 *b, u16 *c, u16 *d) {
    *a = next(script);
    *b = next(script);
    *c = next(script);
    *d = next(script);
    return d;
}

u16 *__cdecl WindowScript_ReadPair(u8 *script, u16 *a, u16 *b) {
    *a = next(script);
    *b = next(script);
    return b;
}
}
