#include "mmxfill.h"

#include <cstdint>
#include <cstring>

extern "C" {

void __cdecl Mmx_FillRows(s32 width, u32 rows, u8 *buffer, s32 pitch) {
    u8 pattern[8];
    std::memcpy(pattern, reinterpret_cast<const void *>((uintptr_t)0x0077e580), 8);
    do {
        u8 *to = buffer;
        s32 left = width;
        do {
            std::memcpy(to, pattern, 8);
            std::memcpy(to + 8, pattern, 8);
            to += 0x10;
            left -= 8;
        } while (left > 0);
        buffer += pitch;
    } while (--rows);
}
}
