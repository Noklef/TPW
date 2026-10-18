// A block of vertices kept four abreast: reimplementation of 0x53ddd0 (its one caller, 0x53df30, adds the corners of the scene's
// generated faces). Its arrays: positions at +0x64 (0x30 bytes a group of four: x, y, z each four floats), normals at +0x68 (three floats
// a vertex) and texture coordinates at +0x6c (0x20 bytes a group: u, v each four floats); the count of vertices at +0x9c.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x53ddd0 (thiscall, pops 0x1c; name unconfirmed): adds a vertex at the count, which goes up by one. With wave not 0, y is raised by
// the ground's height at (x, z) (Terrain_HeightAt) and by the sine table's entry for (the float at 0x874fc0 + z + x) * its scale, and the
// normal is (0, (that entry + 1) / 2, 0); else the normal is the three floats at normal, or (0, 1, 0) for none. Each float is stored by
// the x87 (a signalling NaN quietened).
void __fastcall VertexBlock_Add(u8 *block, void *, float x, float y, float z, float u, float v, u32 wave, const float *normal);

// 0x53e530 (thiscall (positions, unused, uvs, normals), pops 0x10; name unconfirmed; 0x53dc4f adds the scene's generated quads with it): the
// four corners i = 0-3 added (VertexBlock_Add, no wave): x, y, z the three floats at positions + 0xc * i, u and v the first two of
// uvs + 0xc * i, the normal at normals + 0xc * i. Then two triangles of the count before (base) in the index list [+0x94] (8 bytes an
// entry, its words +2, +4, +6 written, +0 left): (base + 2, base + 1, base) and (base + 1, base + 2, base + 3), the list stepping 8 and its
// count +0x98 one up after each. Returns the new +0x98.
u32 __fastcall VertexBlock_AddQuad(u8 *block, void *, const u32 *positions, u32 unused, const u32 *uvs, const u8 *normals);
}
