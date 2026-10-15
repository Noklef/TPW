#include "wtraits.h"

#include <cstdint>

extern "C" {

u16 *__cdecl WTraits_Move(u16 *dst, const u16 *src, u32 n) {
    if ((uintptr_t)src < (uintptr_t)dst && (uintptr_t)dst < (uintptr_t)(src + n)) {
        for (u32 i = n; i != 0; --i) dst[i - 1] = src[i - 1];
    } else {
        for (u32 i = 0; i != n; ++i) dst[i] = src[i];
    }
    return dst;
}
}
