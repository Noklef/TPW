// The software rasteriser's triangle set-ups in cseg (hand-written MMX, 0x6ef7f0-0x6f136d), cdecl (v0, v1, v2). A vertex: x and y 28.4
// fixed at +0 and +4, two depth words at +8, four colour bytes at +0xc, u and v at +0x10 and +0x14 (texels 12.20 fixed once << 20).
// Each sorts the vertices by row ((y - 8) >> 4: [0xfbe0b0] + 4 * vertex), sets the two parts' row counts ([0xfbe204] top to middle,
// [0xfbe200] middle to bottom), the edges' x and slope (records 0xfbe2a8 top to middle, 0xfbe274 middle to bottom, 0xfbe240 top to
// bottom, 0x34 apart: x 17.15 less half a pixel, prestepped by the top's row fraction [0xfbe0c0] or the middle's [0xfbe0bc], the
// prestep at +0x30), the edge pairs [0xfbe208] and [0xfbe210] from [0xfbe6f0] / [0xfbe6f8] + [0xfbe710 + 4 * order], the row
// pointers ([0xfbe024] and [0xfbe028] plus the top row times the pitches, PMADDWD), the gradients (u and v per pixel [0xfbe138] and per
// row [0xfbe140], 1 / area as 2^39 / the cross product, 0 at 0x80 or less), the left edges' start and steps, then ends in the span
// routine [0xfbe044] (render/rasterspans), which the original jumps to with the row pointer in EBP. Names invented (unconfirmed).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6ef7f0: shaded (four shade words per edge, steps [0xfbe150] per pixel and [0xfbe158] per row) with depth ([0xfbe148] per pixel,
// [0xfbe14c] per row; [0xfbe168] the pixel step, [0xfbe148] then with its halves swapped).
void __cdecl RasterTriangle_Draw(const u8 *v0, const u8 *v1, const u8 *v2);

// 0x6f02e0: as RasterTriangle_Draw, one shade for the triangle (v0's colour bytes << 7 to the words at 0xfbe000) and the edges' shade
// left as it is.
void __cdecl RasterTriangle_DrawFlat(const u8 *v0, const u8 *v1, const u8 *v2);

// 0x6f0a50: as RasterTriangle_Draw without depth: [0xfbe148], [0xfbe168] and the edges' depth not written.
void __cdecl RasterTriangle_DrawNoZ(const u8 *v0, const u8 *v1, const u8 *v2);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x88], gRastertriangle_fbe0b0, 0x00fbe0b0, 0x88);
GAME_DATA(u8[0x2a0], gRastertriangle_fbe240, 0x00fbe240, 0x2a0);
GAME_DATA(u8[0x110], gRastertriangle_fbe6f0, 0x00fbe6f0, 0x110);
