// The copy MSVC6 inlines for memcpy and char_traits::copy: rep movsd, then rep movsb for the
// remainder. It runs forward a dword at a time, so where source and destination overlap the
// result is the original's, not memmove's.
#pragma once

#include <cstdint>
#include <cstring>

#include "../../../re/types.h"

inline void Rep_Movs(void *to, const void *from, u32 n) {
    u8 *t = static_cast<u8 *>(to);
    const u8 *f = static_cast<const u8 *>(from);
    for (u32 i = n >> 2; i; --i, t += 4, f += 4) {
        u32 v;
        std::memcpy(&v, f, 4);
        std::memcpy(t, &v, 4);
    }
    for (u32 i = n & 3; i; --i) *t++ = *f++;
}
