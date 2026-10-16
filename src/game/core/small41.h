// A fixed-point 2 x 2 transform and a share scaled by the things' sum: reimplementation of 0x50f310 and 0x503980. Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x50f310 (thiscall on the matrix {+0x20 a, +0x24 b, +0x28 c, +0x2c d: 16.16 fixed point}; out pair, in pair, pops 8): out[0] = (in[1] * c >> 16) + (in[0] * a >> 16) and
// out[1] = (in[1] * d >> 16) + (in[0] * b >> 16), each product taken at 64 bits and cut to the bits 16 to 47.
void __fastcall Matrix2_Transform(u8 *matrix, void *, s32 *out, const s32 *in);

// 0x503980 (thiscall on the table; index, pops 4): 0 when Global7852f8_Times78548c is 0, else the dword at table + (index + 0x141) * 16 times (Things_SumChain * 100 / that
// total, unsigned), divided by 100 (unsigned).
u32 __fastcall Table_ScaledBySum(u8 *table, void *, s32 index);

}  // extern "C"
