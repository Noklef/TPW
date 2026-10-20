// A pair of 32-bit ints (8 bytes; copied in 151 places), x then y, at times 16.16 fixed point:
// reimplementation of 0x4d99c0, 0x4d97b0, 0x4d9cc0, 0x50e080, 0x50ece0 and 0x50f1a0-0x50f1f0 (thiscall),
// and 0x50f850, 0x5104b0 and 0x5104e0 (cdecl, returning a pair through a hidden &out); 0x50ce20 (an atexit entry).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d99c0 (from): the copy constructor, first then second (read after the first is written).
// Returns pair.
u32 *__fastcall IntPair_Copy(u32 *pair, void *, const u32 *from);

// 0x4d97b0 (&out, scale): out = both times scale (32-bit products). Returns out.
u32 *__fastcall IntPair_Scaled(const u32 *pair, void *, u32 *out, u32 scale);

// 0x4d9cc0 (&out, angle; name unconfirmed): out = the pair turned by angle % 360 (a signed remainder): 90 or -270 (y, -x), 180 or -180
// (-x, -y), 270 or -90 (-y, x), both read before out is written; any other (0, or one reported to the empty Debug_Printf) a copy, first
// then second (read after the first is written). Returns out.
u32 *__fastcall IntPair_Rotated(const u32 *pair, void *, u32 *out, s32 angle);

// 0x50e080 (&out, divisor): out = each << 16 (as 64 bits) / divisor, y first. The original's
// IDIV faults on a 0 divisor or a quotient past 31 bits; this one wraps the quotient instead.
// Returns out.
s32 *__fastcall IntPair_DivFixed(const s32 *pair, void *, s32 *out, s32 divisor);

// 0x50ece0 (&out, other): out = pair + other (both read before out is written). Returns out.
u32 *__fastcall IntPair_Add(const u32 *pair, void *, u32 *out, const u32 *other);

// 0x50cd80 (&out, other): out = pair - other (both read before out is written). Returns out.
u32 *__fastcall IntPair_Sub(const u32 *pair, void *, u32 *out, const u32 *other);

// 0x50df70 (&out, scale): out = each times scale in 16.16 fixed point (the 64-bit product's
// bits 16-47), y first (both read before out is written). Returns out.
s32 *__fastcall IntPair_MulFixed(const s32 *pair, void *, s32 *out, s32 scale);

// 0x4d49a0 (cdecl): the square root of n (an unsigned 32-bit value) on the x87 at the game's
// precision, truncated (__ftol); the pair lengths take it of x * x + y * y.
u32 __cdecl Math_ISqrt(u32 n);

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

// 0x50f600 (&out, &pair, limit): the pair cut to the limit's length (16.16). Its length is the one the steering inlines: Math_ISqrt of x * x + y * y
// (32-bit products) while both magnitudes are under 0x1000, else of the magnitudes shifted right (logically) by 8 (under 0x400000) or 18, shifted back.
// Below the limit (signed): out = pair; else out = pair * (limit << 16 / length) (16.16 products; one IDIV that faults, sic, on a length of 0 or a
// quotient past 31 bits). Returns out.
s32 *__cdecl IntPair_Truncate(s32 *out, const s32 *pair, s32 limit);

// 0x50f700 (&out, &pair): the pair made 1.0 long. While both magnitudes are under 0x1000: root = Math_ISqrt((x * x + y * y) << 6), s = 0x80000 /
// root (0 for a root of 0) and out = (x * s, y * s) (32-bit products); else root = Math_ISqrt of the magnitudes shifted right (arithmetically) by 8
// (under 0x400000) or 18, squared and summed, s = 0x1000000 / root or 0x4000 / root (0 for 0) and out = pair * s (16.16 products, y first). Returns out.
s32 *__cdecl IntPair_Normalize(s32 *out, const s32 *pair);
// 0x50ce20 (no arguments): the atexit entry of the pair 0x7cf100 (zeroed by the static initialiser 0x50ce00, which registers it): nothing
// to destroy, a bare RET. The name is unconfirmed.
void __cdecl IntPair_ZeroAtExit();

}
