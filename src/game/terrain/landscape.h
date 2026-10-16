// The landscape renderer's accessors: reimplementation of 0x5856e0, 0x585710, 0x585ef0 and
// 0x585f00.
//
// The landscape renderer is the object at 0x8c1828 (its grids from +0x360 and +0x1760, the size
// at +0x21e0 / +0x21e4; 0x5856c0 sets its +0x1f68 to 300.0 and rebuilds it). +0x1f70 holds four
// points of three floats that the weather sets (0x57da4e), +0x1fa0 a point the terrain drawing
// reads (0x570ddd), +0x1fd0 a pointer it walks (0x570e50) and +0x2ec8 the detail level (from
// the option 0x785888, 0x44da17; 1 or 4 to start with, 0x584f64).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5856e0 (thiscall; i, x, y, z, floats moved as dwords): point i at +0x1f70 + 12 i.
void __fastcall Landscape_SetPoint(u8 *landscape, void *, s32 i, u32 x, u32 y, u32 z);

// 0x585710 (thiscall; level): +0x2ec8, the detail level.
void __fastcall Landscape_SetDetail(u8 *landscape, void *, u32 level);

// 0x585ef0 (thiscall): +0x1fd0.
u32 __fastcall Landscape_List(const u8 *landscape);

// 0x585f00 (thiscall): the point at +0x1fa0.
u8 *__fastcall Landscape_Point(u8 *landscape);
}
