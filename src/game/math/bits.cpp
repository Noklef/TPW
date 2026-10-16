#include "bits.h"

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
}
