// The landscape renderer's constructor (0x584ef0), accessors and texture releases: reimplementation of 0x5856e0, 0x585710, 0x585ef0, 0x585f00, 0x585e20
// and 0x5851d0.
//
// The landscape renderer is the object at 0x8c1828 (its grids from +0x360 and +0x1760, the size
// at +0x21e0 / +0x21e4; 0x5856c0 sets its +0x1f68 to 300.0 and rebuilds it). +0x1f70 holds four
// points of three floats that the weather sets (0x57da4e), +0x1fa0 a point the terrain drawing
// reads (0x570ddd), +0x1fd0 a pointer it walks (0x570e50) and +0x2ec8 the detail level (from
// the option 0x785888, 0x44da17; 1 or 4 to start with, 0x584f64).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x584ef0 (thiscall; the constructor of 0x8c1828, run by the static initializer 0x584ec0; name unconfirmed): +0x2ecc an identity
// matrix, +0x21e8 = 24000.0, the size +0x21e0 / +0x21e4 = 0x960, +0x1f68 = 300.0, the detail level +0x2ec8 = 1 when [0x8bd524] is 4,
// else 4, +0x1f64 and +0x1f6c = 0; for i 0-3 the point at +0x1fa0 + 12 i zeroed and the weather's at +0x1f70 + 12 i = (0.0008 +
// 0.0005 i, 0, 0.00009 - 0.00003 i) and +0x21ec + 4 i = (7 - i) * pi * 0.00893 (x87, the constants at 0x701f38-0x701f48 and
// 0x701ee0); the texture arrays +0x1fd0-+0x1fd8 = 0; from +0x2c88 and +0x2da8 the index words of 8 x 3 cells (two triangles each:
// a, a + 5, a + 4, a, a + 1, a + 5 with a = 4 row + column, 0x24 more in the second); +0x21dc = 0, +0x2f0c = 0xcfffffff and +0x2f10 =
// 0x38000000. Returns the landscape.
u8 *__fastcall Landscape_Init(u8 *landscape);

// 0x5856e0 (thiscall; i, x, y, z, floats moved as dwords): point i at +0x1f70 + 12 i.
void __fastcall Landscape_SetPoint(u8 *landscape, void *, s32 i, u32 x, u32 y, u32 z);

// 0x585710 (thiscall; level): +0x2ec8, the detail level.
void __fastcall Landscape_SetDetail(u8 *landscape, void *, u32 level);

// 0x585ef0 (thiscall): +0x1fd0.
u32 __fastcall Landscape_List(const u8 *landscape);

// 0x585f00 (thiscall): the point at +0x1fa0.
u8 *__fastcall Landscape_Point(u8 *landscape);

// 0x585e20 (thiscall; name unconfirmed): the three texture arrays at +0x1fd0, +0x1fd4 and +0x1fd8, each when set: its first texture
// released (Texture_Release), the array destroyed when still set (Texture_DestroyArray), the field cleared. Returns 1.
u32 __fastcall Landscape_FreeTextures(u8 *landscape);

// 0x585ce0 (thiscall; name unconfirmed): with a texture at +0x1fd4, 16 x 16 colours sampled from its pixels (Texture_Lock) into
// +0xf60: step = its s16 sizes at +0x10 / +0x12 over 16 (signed), from half a step in; the colour of sample (x, y) at +0xf60 + 4 y +
// 0x40 x is the pixel at the s16 pitch +0x34 times the row plus the column: format ([0x8bd554]) 2 a u16 c turned into (c & 0xff80) << 12
// | (c & 0xf0) << 8 | (c & 0xf) << 4 | 0xff080808 (sic), 3 a u32 as it is, else the dword there kept; then copied to +0x400, +0x800 and
// +0xc00 from it.
void __fastcall Landscape_SampleColours(u8 *landscape);

// 0x5851d0 (thiscall; textures; name unconfirmed): the 0x20-byte entries from +0x1fdc (their texture array first), the count at
// +0x21dc. The entry holding textures is looked for from entry[count] (one past the last, sic) down to the first: none (or a
// negative count) 0. Found, its array is cleared, released and destroyed when set, the count lowered and, unless that leaves 0,
// the entry swapped with entry[count] (the new last; entry[-1] when the count was 0, sic). Returns 1.
u32 __fastcall Landscape_RemoveTexture(u8 *landscape, void *, u8 *textures);

// 0x418d70 (thiscall on a list of texture arrays; name unconfirmed): each of the first [0x785724] (the count re-read after each) removed
// from the landscape at 0x8c1828 (Landscape_RemoveTexture). Returns the count last read.
u32 __fastcall Landscape_RemoveTextureList(u8 **list);
}
