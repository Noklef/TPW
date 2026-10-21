#include "refblock.h"

#include <cstdint>

extern "C" {

void __fastcall RefBlock_Copy16(const u8 *source, s32 swapped, s32 pitch, u8 *out, u8 bias) {
    static const u8 kSwapped[8] = {2, 7, 6, 11, 10, 15, 14, 19}, kStraight[8] = {3, 2, 7, 6, 11, 10, 15, 14};
    const u8 *order = swapped ? kSwapped : kStraight;
    for (s32 row = 0; row < 8; ++row, source += pitch * 4, out += 0x40)
        for (s32 k = 0; k < 8; ++k) out[2 + k * 4] = (u8)(source[order[k]] + bias);
}

void __fastcall RefBlock_Copy8(s32 base, s32 offset, s32 pitch, u8 *out, u8 bias) {
    const u8 *source = reinterpret_cast<const u8 *>((uintptr_t)(u32)(base + offset));
    for (s32 row = 0; row < 8; ++row, source += pitch * 8, out += 0x20)
        for (s32 k = 0; k < 8; ++k) out[2 + k * 4] = (u8)(source[k * 4] + bias);
}
}
