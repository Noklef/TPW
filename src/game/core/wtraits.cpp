#include "wtraits.h"

#include <cstdint>

#include "crt.h"

extern "C" {

u16 *__cdecl WTraits_Move(u16 *dst, const u16 *src, u32 n) {
    if ((uintptr_t)src < (uintptr_t)dst && (uintptr_t)dst < (uintptr_t)(src + n)) {
        for (u32 i = n; i != 0; --i) dst[i - 1] = src[i - 1];
    } else {
        for (u32 i = 0; i != n; ++i) dst[i] = src[i];
    }
    return dst;
}

u16 *__cdecl WTraits_Copy(u16 *dst, const u16 *src, u32 n) {
    // A character at a time, forward: where dst overlaps src from above, what it copies repeats
    // (volatile, so it is not made a memmove).
    const volatile u16 *from = src;
    for (u32 i = 0; i != n; ++i) dst[i] = from[i];
    return dst;
}

u16 *__cdecl WTraits_Assign(u16 *dst, u32 ch, u32 n) {
    for (u32 i = 0; i != n; ++i) dst[i] = (u16)ch;
    return dst;
}

void __fastcall ImeWString_SetLength(u8 *string, void *, u32 length) {
    *reinterpret_cast<u32 *>(string + 8) = length;
    (*reinterpret_cast<u16 **>(string + 4))[length] = 0;
}

u32 __cdecl WTraits_Length(const u16 *text) { return Crt().wcslen(text); }
}
