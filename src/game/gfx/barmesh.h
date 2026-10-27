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

// 0x477870 (thiscall (name), pops 4; name unconfirmed): the bar built: the GUI dialog base (GuiDialog_Ctor with the name), vtable
// 0x6fee88, its record (+0xbc; one name, the bar's at +0xb0) and animator (+0x6c; one node at +0x3c: 4 + 8 vertices, 8 normals, the
// matrix at +0xdc) made into a mesh (Animator_Create with flags 1; on a failure, which neither call has, bit 0 of +0xbc cleared and the
// bar returned as it is), the part's matrix the identity; then its two quads: vertices 0 (0, 100), 1 (100, 100), 2 (100, 0), 5 (100,
// level), 3 and 4 (0, level), 6 (100, 0), 7 (0, 0) (z 0, the level +0x120 set to 0 first), the normals (0, 0, -1), the indices 0 1 2
// 2 3 0 and 4 5 6 6 7 4, the texture block (the record's table +0xd4; 0, 4, 0, 7) and the coordinates (v level * 0.01 on the top
// edge); +4 = 1. Returns the bar.
u8 *__fastcall BarMesh_Construct(u8 *bar, void *, const char *name);

// 0x477ff0 (thiscall (name), pops 4; name unconfirmed): the same for the sibling overlay (vtable 0x6feeb8), its quads lying the other
// way: vertices 0 (0, 100), 6 (100, 100), 1 (0, 100), 4 (level, 100), 2 and 5 (level, 0), 7 (100, 0), 3 (0, 0), the indices 0 1 2 2 3
// 0 and 4 6 7 7 5 4, the coordinates u level * 0.01. Returns the mesh.
u8 *__fastcall MeshOverlay_Construct(u8 *mesh, void *, const char *name);

// 0x477d70 (virtual, slot 5 of both vtables: the dialog's word +8; thiscall (word), pops 4; name unconfirmed): nothing when the word
// is +8's already; else the animator (+0x6c) wakes (Animator_Wake) for a word other than 0 and sleeps (Animator_Sleep) for 0, and +8
// takes the word.
void __fastcall BarMesh_SetShown(u8 *mesh, void *, u32 shown);

// 0x477db0 (virtual, slot 3 of 0x6fee88: thiscall (flags, desc), pops 8; name unconfirmed): nothing without a holder (+4), a desc or
// flags. Flag 0x10: the desc's rectangle (the four words at +4 of its +0x14) to +0x18 and again to +0x24; flag 4: the desc's word +8
// to +0x30. Flags 0x14: the matrix at +0xdc put on the screen (Sprite_ScreenMatrix) from the bounds +0x4c (low) and +0x58 (high), x and
// y each swapped when high is below low or unordered (the new low through the x87), as Overlay_Apply does (+0x18, +0x1e, the widths of
// the rectangle, low, high - low, +0x34, +0x36, +0x30). Flag 2: the desc's word +6 to +0x38 and the scale 1 - it / 40. The scale +0x114
// is written back in any case, as read before the matrix (whose element 14 it is, sic). Flag 8: the level the desc's dword / 1024 * 100,
// at most 100 and at least 0; unless equal to +0x120 (or that is a NaN) it goes there and the bar's top edge follows (slots 9 and 10:
// BarMesh_SetTopVertices, BarMesh_SetTopTexCoords). After a matrix or scale change the holder (+0x90) gets the matrix (Holder_SetMatrix);
// after any change the bit 31 of 0x8bd508 is set.
void __fastcall BarMesh_Apply(u8 *bar, void *, u32 flags, const u8 *desc);

// 0x478400 (virtual, slot 3 of 0x6feeb8; name unconfirmed): the same for the sibling mesh overlay, its new level put on its quads in place
// of the slot calls: vertices 1 and 4 (level, 100, 0), 2 and 5 (level, 0, 0); coordinates 1 (u, 1), 4 and 2 (u, 0.5), 5 (u, 0), u = the
// level * 0.01 (the pointers and the level read again as it goes).
void __fastcall MeshOverlay_Apply(u8 *mesh, void *, u32 flags, const u8 *desc);
}
