// A pair of 32-bit ints (8 bytes; copied in 151 places), x then y, at times 16.16 fixed point:
// reimplementation of 0x4d99c0, 0x4d97b0, 0x50e080, 0x50ece0 and 0x50f1a0-0x50f1f0 (thiscall),
// and 0x50f850, 0x5104b0 and 0x5104e0 (cdecl, returning a pair through a hidden &out).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d99c0 (from): the copy constructor, first then second (read after the first is written).
// Returns pair.
u32 *__fastcall IntPair_Copy(u32 *pair, void *, const u32 *from);

// 0x4d97b0 (&out, scale): out = both times scale (32-bit products). Returns out.
u32 *__fastcall IntPair_Scaled(const u32 *pair, void *, u32 *out, u32 scale);

// 0x50e080 (&out, divisor): out = each << 16 (as 64 bits) / divisor, y first. The original's
// IDIV faults on a 0 divisor or a quotient past 31 bits; this one wraps the quotient instead.
// Returns out.
s32 *__fastcall IntPair_DivFixed(const s32 *pair, void *, s32 *out, s32 divisor);

// 0x50ece0 (&out, other): out = pair + other (both read before out is written). Returns out.
u32 *__fastcall IntPair_Add(const u32 *pair, void *, u32 *out, const u32 *other);

// 0x50f1a0 (&out): out = -pair. Returns out.
u32 *__fastcall IntPair_Neg(const u32 *pair, void *, u32 *out);

// 0x50f1c0: an octagonal length, |x| + |y| - min(|x|, |y|) / 2 (an arithmetic shift; |INT_MIN|
// stays negative).
s32 __fastcall IntPair_ApproxLength(const s32 *pair);

// 0x50f1f0: (0, 0). Returns pair.
u32 *__fastcall IntPair_Ctor(u32 *pair);

// 0x50f850 (&out, &pair): out = (-y, x), the pair turned a quarter (y read first). Returns out.
u32 *__cdecl IntPair_Perp(u32 *out, const u32 *pair);

// 0x5104b0 / 0x5104e0 (&out, x, y): out = (x >> 16, y >> 16) (logical shifts) / (x << 16,
// y << 16). Return out.
u32 *__cdecl IntPair_FromFixed(u32 *out, u32 x, u32 y);
u32 *__cdecl IntPair_ToFixed(u32 *out, u32 x, u32 y);
}
