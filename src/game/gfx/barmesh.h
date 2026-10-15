// A bar drawn as a mesh (vtable 0x6fee88, built at 0x477870): reimplementation of 0x477c00, 0x4786f0
// and its virtuals 0x477cc0 and 0x477d20. Its vertices (+0x7c) are kept four to a 0x30-byte block
// (x[4], y[4], z[4]), its texture coordinates (+0x84) four to a 0x20-byte block (u[4], v[4]);
// +0x120 is the bar's level (its top edge's y).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x477c00: vertex i of verts to (x, y, z) (block i / 4, lane i % 4). Each goes through the
// x87 as a float.
void __cdecl BarMesh_SetVertex(float *verts, u32 i, float x, float y, float z);

// 0x4786f0: texture coordinate i of coords to (u, v) (block i / 4, lane i % 4), the same way.
void __cdecl BarMesh_SetTexCoord(float *coords, u32 i, float u, float v);

// 0x477cc0 (virtual, slot 9): the top edge's vertices at the level: 2 and 5 to (100, level,
// 0), 3 and 4 to (0, level, 0). The vertex pointer is read again for each.
void __fastcall BarMesh_SetTopVertices(const u8 *bar);

// 0x477d20 (virtual, slot 10): the top edge's texture coordinates at v = level * 0.01: 3 and
// 5 to (0.5, v), 2 to (1, v), 4 to (0, v). The pointer is read again for each.
void __fastcall BarMesh_SetTopTexCoords(const u8 *bar);
}
