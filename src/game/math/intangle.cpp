#include "intangle.h"

#include <cstdint>

extern "C" {

u32 __cdecl IntAngle_Atan2(s32 dx, s32 dy) {
    u32 a = (u32)dx, b = (u32)dy;
    if (a == 0 && b == 0) return 0;
    const bool xNeg = dx < 0, yNeg = dy < 0;
    if (xNeg) a = 0u - a;
    if (yNeg) b = 0u - b;
    const bool steep = (s32)a < (s32)b;  // (JL on the magnitudes)
    const u32 q = steep ? (a << 8) / b : (b << 8) / a;
    const u16 t = *reinterpret_cast<const u16 *>((uintptr_t)(0x006e71b4u + q * 2));
    // (negate t, then add) by quadrant and steepness
    static const u16 kAdd[2][2][2] = {{{0x200, 0x400}, {0x200, 0x000}}, {{0x600, 0x400}, {0x600, 0x800}}};
    static const bool kNeg[2][2][2] = {{{false, true}, {true, false}}, {{true, false}, {false, true}}};
    const u16 r = (u16)((kNeg[xNeg][yNeg][steep] ? (u16)(0u - t) : t) + kAdd[xNeg][yNeg][steep]);
    return (q & 0xffff0000u) | (r & 0x7ffu);
}
}
