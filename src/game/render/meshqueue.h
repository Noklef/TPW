// The renderer's two mesh queues: reimplementation of 0x579c70, 0x57c260 and 0x57c270.
//
// Both hold up to 0x100 meshes found while the scene is walked (0x57a7a0, 0x57a91f) and are run
// by the renderer's frame (0x576c55, 0x576d90). The first (0x8bb220, the count after the entries
// at +0x400) has each mesh flagged 0x10 and drawn through its virtual base (0x579c80); the second
// (0x8bb638, the count first and the entries from +4) takes the meshes whose material has 0x800
// and runs 0x57be90 on each, emptying itself (0x57c2a0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x579c70 (thiscall, the first queue): emptied (+0x400 = 0).
void __fastcall MeshQueue_Clear(u8 *queue);

// 0x57c260 (static initialiser): the second queue (0x8bb638) emptied.
void __cdecl MeshQueue2_Init();

// 0x57d1d0 (thiscall, a 0xa1bc-byte render object, built by 0x57d13e): its defaults. +0 = 1, +4 and
// most of the floats around +0x3308-+0x3354 and +0xa16c-+0xa1b8 set (1.0f diagonal of a matrix
// at +0x9d58), the rest of the object left as it was. Returns the object.
u8 *__fastcall Render3d_Init(u8 *object);

// 0x57c270 (thiscall, the second queue; mesh): appended when there is room; 1, or 0 when full.
u32 __fastcall MeshQueue2_Add(u32 *queue, void *, u32 mesh);
}
