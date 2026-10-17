#include "bits.h"

#include <cstdint>

extern "C" {

u32 __cdecl Bits_Count(u32 mask) {
    u32 count = 0;
    for (; mask; mask >>= 1)
        if (mask & 1)
            ++count;
    return count;
}

u32 __cdecl Bits_Shift(u32 mask) {
    u32 shift = 0;
    if (mask == 0)
        return 0;
    for (; (mask & 1) == 0; mask >>= 1)
        ++shift;
    return shift;
}

s32 __cdecl DivCeil(s32 a, s32 b) { return (s32)((u32)a + (u32)b - 1) / b; }

u32 __cdecl Bits_Reverse(u32 code, s32 length) {
    u32 result = 0;
    do {
        result |= code & 1;
        code >>= 1;
        result <<= 1;
    } while (--length > 0);
    return result >> 1;
}
}
