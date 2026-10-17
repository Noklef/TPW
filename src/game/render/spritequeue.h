// The sprite queue's creation, appends and release: reimplementation of 0x58a430, 0x58a550, 0x58a6b0, 0x58a6e0 and 0x58a490.
//
// The sprite queue is the object at [0x8c47a0]: the sprite frames readied this frame (0x588ef0)
// waiting to be drawn (0x58a770). It has a list of 0x800 at +0 (count +0x2404), a second list at
// +0x2000 (count +0x2408) and the frame sets at +0x2200 (count +0x240c, run by 0x58a720).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x58a550 (thiscall on a frame set's +0x20; _ = the set's address + 0x20): the set is queued
// once for the current stamp (0x8bcc40, 0x8bcc44). If its stamp (+0xc, +0x10) already matches,
// 1. Otherwise the stamp is taken, and the set is appended to the frame sets of the queue at
// [0x8c47a0] when there is room (0x80): 1, or 0 when full.
u32 __fastcall SpriteQueue_Ready(u8 *setPlus20);

// 0x58a6b0 (thiscall; frame): appended to the first list when there is room; 1, or 0 when full.
u32 __fastcall SpriteQueue_Add(u8 *queue, void *, u32 frame);

// 0x58a6e0 (thiscall; frame): the same for the second list. Its limit is also 0x800 but it has
// room for 0x80 (sic): past that it runs over the frame sets, and its 0x102nd entry lands on its
// own count.
u32 __fastcall SpriteQueue_Add2(u8 *queue, void *, u32 frame);

// 0x58a490 (cdecl; name unconfirmed): with a queue, its texture array at +0x2400 (when set) destroyed (Texture_DestroyArray, gfx/texture)
// and cleared, the queue freed (GameHeap_Free) and [0x8c47a0] cleared: 1. Without one 0.
u32 __cdecl SpriteQueue_Free();

// 0x58a430 (cdecl; name unconfirmed): without a queue at [0x8c47a0], a new one of 0x2410 bytes (the game's heap) there, its counts
// (+0x2404, +0x2408, +0x240c) and texture array (+0x2400) cleared: 1. 0 when there was one, or when the allocation failed (left null, the
// error (3, 0xb0754000) pushed onto the engine's list, EngineErrors_Push).
u32 __cdecl SpriteQueue_Create();
}
