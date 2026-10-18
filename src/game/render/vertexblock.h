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
}
