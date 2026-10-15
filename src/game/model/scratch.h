// The node vertex scratch buffer: reimplementation of 0x450e60 and 0x450e80. 12 bytes (a
// vector) for each vertex of the biggest node loaded: the model loader raises the count at
// 0x79f7d0, the buffer is at 0x79f7e0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x450e60: 0x79f7e0 = GlobalMem_Alloc(12 * count (0x79f7d0), zeroed).
void __cdecl Model_AllocVertexScratch();

// 0x450e80: frees 0x79f7e0 and clears it.
void __cdecl Model_FreeVertexScratch();

// 0x450f20: the world positions of the view's (0x79fb18's) node vertices into the buffer, one
// after another: the node at view +0x2c, then, unless only that one, each of the model's
// (view +4) parts (u16 count +0x48, from node u16 +0x46: of the u16 +0x44 big nodes at +0x70,
// 0xa0 bytes each, then the small ones at +0x74, 0x58 bytes) whose flags (+0x7c, 0x14 bytes
// each) have 2, each part's instance (view +0x28 -> +4, 0x14 bytes each) taking its first
// vertex's index at +0x10. A node's u16 +0x58 vertices (grouped at +0x60: x, y, z 0x10 apart
// in 0x30-byte groups) are offset by its +0x40 vector and those of its ancestors but the root
// (none for a node without a parent), then put about the root's +0x40 vector (model +0x78)
// turned by the view's angle (+0x10: 90, 180 or 270; other angles none).
void __cdecl Model_FillVertexScratch(const u8 *view, s32 mainOnly);

// 0x450ea0: makes view current (0x79fb18) and picks how its floor is found (0x79fb10): 2,
// with all its vertices filled, when its holder (+0x28) has flag 8; else 1 when its model's
// (+4) +0x30 has 0x80 or its +0xa0 is not 0; else (0, or NaN) 4, with the main node's filled.
void __cdecl Model_BeginView(const u8 *view);
}
