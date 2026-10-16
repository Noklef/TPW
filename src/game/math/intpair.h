// A pair of 32-bit ints (8 bytes; copied in 151 places): reimplementation of 0x4d99c0 and
// 0x4d97b0 (thiscall).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d99c0 (from): the copy constructor, first then second (read after the first is written).
// Returns pair.
u32 *__fastcall IntPair_Copy(u32 *pair, void *, const u32 *from);

// 0x4d97b0 (&out, scale): out = both times scale (32-bit products). Returns out.
u32 *__fastcall IntPair_Scaled(const u32 *pair, void *, u32 *out, u32 scale);
}
