// The video library's macroblock decoder (16x16 blocks of four 8x8 luma squares and two chroma values, the EA TGQ kind: unconfirmed).
// Colours come from the tables built at run time: luma 0xfb7930, the two chroma ones 0xfb7d30 / 0xfb7b30 (128 dwords each, summed), each
// indexed by (value * [0xfb7820]) (a 16.16 product, rounded by its bit 15) >> 1 & 0x7f. Reimplementation of 0x671340 and 0x671650. The
// names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x671340 (cdecl (y0, y1, y2, y3, c0, c1, out, pitch: the six values signed bytes); name unconfirmed): a flat macroblock at 16 bits a
// pixel: each 8x8 square (y0 at out, y1 16 bytes right, y2 pitch * 8 down, y3 both) filled with luma[y] + chroma 0xfb7d30[c0] +
// 0xfb7b30[c1] (its low 16 bits), the rows (pitch >> 1) * 2 bytes apart.
void __cdecl TgqVideo_FillBlocks16(s32 y0, s32 y1, s32 y2, s32 y3, s32 c0, s32 c1, u8 *out, u32 pitch);

// 0x671650 (cdecl, as TgqVideo_FillBlocks16; name unconfirmed): the same at 32 bits a pixel: the squares 32 bytes right and pitch * 8
// down, the rows (pitch >> 2) * 4 bytes apart.
void __cdecl TgqVideo_FillBlocks32(s32 y0, s32 y1, s32 y2, s32 y3, s32 c0, s32 c1, u8 *out, u32 pitch);

// 0x66fd70 (fastcall (r, g; b, unused, red bits, green bits, blue bits, red shift, green shift, blue shift), pops 32; name unconfirmed):
// a colour packed twice into a pixel pair: each channel v rounded to its bits ((v + (1 << (7 - bits))) / (1 << (8 - bits)), signed;
// below 0 (v - that) / it), clamped to 1 - (1 << bits) .. (1 << bits) - 1, into the low half at its shift; and truncated (v / (1 << (8 -
// bits))) into the high half at shift + 16, after a borrow chain: a negative rounded blue takes one from the rounded green, that from
// the rounded red, that from the truncated blue, that from the truncated green and that from the truncated red. Each value is masked to
// its bits; with the bits not summing to 16 both halves get their top bit (0x80008000). Shift counts are taken mod 32, as the CPU does.
u32 __fastcall TgqVideo_PackColour(s32 r, s32 g, s32 b, u32 unused, u32 red_bits, u32 green_bits, u32 blue_bits, u32 red_shift,
                                   u32 green_shift, u32 blue_shift);

// 0x66fc10 (fastcall (unused, red bits; green bits, blue bits, red shift, green shift, blue shift), pops 20; name unconfirmed): the
// colour tables for a pixel format (TgqVideo_PackColour): for i 0-127 with c = (s8)(2i + 1), luma 0xfb7930[i] = (c + 0x80) as grey,
// 0xfb7d30[i] = (0, c * -0.3441, c * 1.7720) and 0xfb7b30[i] = (c * 1.4020, c * -0.7141, 0), the products 16.16 rounded by bit 15.
void __fastcall TgqVideo_BuildTables(u32 unused, u32 red_bits, u32 green_bits, u32 blue_bits, u32 red_shift, u32 green_shift,
                                     u32 blue_shift);

// 0x671000 (cdecl (quality); name unconfirmed): the dequantiser 0xfb7820 (64 dwords) from the base matrix 0x705c88: with d = 100 -
// quality, base = d * 22 / 100 + 8 and step = d * 50 / 100 + 10 - base (signed, truncated), entry (row, k) = the matrix's * ((row + k)
// * step / 14 + base) << 16, as 16.16 rounded by bit 15 (the products wrapping).
void __cdecl TgqVideo_SetQuality(s32 quality);
}
