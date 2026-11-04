// The renderer's scrolling texture layers: reimplementation of 0x585f10. Name invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// (Test seam: the fncmp driver's table replaces the calls by address.)
void ScrollLayers_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x585f10 (thiscall; called from the frame at 0x576bfd): nothing without a layer count (+0x2ec8), the textures +0x1fd0, +0x1fd8, for driver kind 4 or with flag 0x80000000 of 0x8bcbc8. Else the first texture kept when 0x8bd508 has 0xc0000000, both touched
// in the cache [0x8c4b44]. The step is [[0x8bcbac] +0x9d98] * 25, 0 outside -25 to 25, for a NaN or when 0x8bd508 has 4. Each layer (12 bytes from +0x1fa0): its u and v (+0, +8) moved by the speeds 0x30 and 0x28 bytes back times the step, x87 at the
// game's precision, and outside -1 to 1 less their whole part. The +0x21dc items at 0x20 bytes from +0x1fdc touched. Returns 1.
u32 __fastcall ScrollLayers_Update(u8 *self);
}
