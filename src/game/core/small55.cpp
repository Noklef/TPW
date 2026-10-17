#include "small55.h"

#include <cstdint>

extern "C" {

u32 *__cdecl Colour_Blend3(u32 *out, s32 position, const u32 *a, const u32 *b, const u32 *c) {
    if (position < 0) position = 0;
    else if (position > 100) position = 100;
    const u32 *from, *to;
    s32 fraction;
    if (position < 0x32) {
        from = a;
        to = b;
        fraction = (position << 16) / 0x32;
    } else {
        from = b;
        to = c;
        fraction = ((position - 0x32) << 16) / 0x32;
    }
    u8 record[36];
    for (u32 i = 0; i < 9; ++i)
        reinterpret_cast<u32 *>(record)[i] = from[i];
    const u8 *start = reinterpret_cast<const u8 *>(from);
    const u8 *end = reinterpret_cast<const u8 *>(to);
    for (u32 k = 0; k < 3; ++k)
        record[k] = (u8)(record[k] + (u8)((((s32)end[k] - (s32)start[k]) * fraction) >> 16));
    record[3] = 0xff;
    for (u32 i = 0; i < 9; ++i)
        out[i] = reinterpret_cast<u32 *>(record)[i];
    return out;
}

}  // extern "C"
