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
}
