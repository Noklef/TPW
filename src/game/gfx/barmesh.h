// A bar drawn as a mesh (vtable 0x6fee88, built at 0x477870): reimplementation of 0x477c00, 0x4786f0,
// its virtuals 0x477cc0 and 0x477d20, and the destructors 0x477c30 / 0x477c50 and those of its sibling
// mesh overlay (vtable 0x6feeb8, built at 0x477ff0; sharing its virtuals 0x477d70 and 0x51ad30)
// 0x478370 / 0x478390. Both keep an animator at +0x6c and a flag dword at +0xbc. Its vertices (+0x7c) are kept four to a 0x30-byte block
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

// 0x477c50 (name unconfirmed): the vtable back to 0x6fee88; with a holder (+4), bit 0 of the dword +0xbc cleared
// (Dword_ClearBit0) and the animator +0x6c disposed of (Animator_Dispose); then GuiDialog_Dtor.
void __fastcall BarMesh_Dtor(u8 *bar);

// 0x477c30 (thiscall (flags), pops 4; slot 0 of 0x6fee88; name unconfirmed): BarMesh_Dtor, the bar freed (Mem_Free) for
// flags bit 0; returns it.
u8 *__fastcall BarMesh_ScalarDtor(u8 *bar, void *, u32 flags);

// 0x478390 / 0x478370 (names unconfirmed): the same for the sibling mesh overlay, its vtable 0x6feeb8.
void __fastcall MeshOverlay_Dtor(u8 *mesh);

u8 *__fastcall MeshOverlay_ScalarDtor(u8 *mesh, void *, u32 flags);
}
