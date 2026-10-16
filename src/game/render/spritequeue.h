// The sprite queue's appends: reimplementation of 0x58a6b0 and 0x58a6e0.
//
// The sprite queue is the object at [0x8c47a0]: the sprite frames readied this frame (0x588ef0)
// waiting to be drawn (0x58a770). It has a list of 0x800 at +0 (count +0x2404), a second list at
// +0x2000 (count +0x2408) and the frame sets at +0x2200 (count +0x240c, run by 0x58a720).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x58a6b0 (thiscall; frame): appended to the first list when there is room; 1, or 0 when full.
u32 __fastcall SpriteQueue_Add(u8 *queue, void *, u32 frame);

// 0x58a6e0 (thiscall; frame): the same for the second list. Its limit is also 0x800 but it has
// room for 0x80 (sic): past that it runs over the frame sets, and its 0x102nd entry lands on its
// own count.
u32 __fastcall SpriteQueue_Add2(u8 *queue, void *, u32 frame);
}
