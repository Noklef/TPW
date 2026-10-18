// The height of a model's floor under a point: reimplementation of 0x451390 (called by
// 0x451290 for the camera's model and each of its parts).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x451390: the highest y, from -100, of the model's floor faces (flag 0x8000) under the
// point (x, z). Faces come in groups (u16 count +0x5a, 16-byte records at +0x6c: first face
// u16 +4, face count u16 +6) of 8-byte faces at +0x70 (flags, then three u16 vertex indices
// through the u16 remap at +0x94 into the shared vertices at [0x79f7e0], from the instance's
// u16 +0x10 if there is an instance). A face is under the point when, for each edge a -> b
// (2 -> 0, 1 -> 2, 0 -> 1), (-(dz a.x + (a.x - b.x) a.z) + dz x) + (a.x - b.x) z >= 0, dz =
// b.z - a.z (a NaN: not under). Its plane's normal is the next of normals (12 bytes each, per
// floor face) if given, else the model's (+0x64) indexed by the flags' low 15 bits, turned by
// the view's angle ([0x79fb18] +0x10: 90, 180 or 270; other angles none): the height is
// -(((-(n.x v0.x + n.y v0.y + n.z v0.z) + n.x x) + n.z z) / n.y), kept when above the best
// so far. With first, the first face under the point ends the search.
float __cdecl Model_HeightAt(const u8 *model, const u8 *instance, float x, float z, const float *normals, s32 first);

// 0x451290: the view's (0x79fb18) floor height under (x, z): unless noMain, its main node's
// (+0x2c, with its normals at +0x30, the first face found), or the root's y (model +4 -> +0x78
// -> +0x44) when that finds none (-100, or NaN); then, unless noParts and when the holder
// (+0x28) has flag 8, the higher of each part flagged 2 (model +0x48 of them from +0x46, all
// taken as big nodes, 0xa0 bytes apart, sic; instances holder +4) whose flags have 4 ends it
// with that height (first found) when it is higher. -100 when nothing is found.
float __cdecl Model_FloorHeight(float x, float z, s32 noParts, s32 noMain);

// 0x4511a0 (cdecl (x, z, &out); name unconfirmed): the floor under (x, z) by the view's way of finding it (0x79fb10, as
// Model_BeginView picks it). A NaN offset (the view's model +4 -> +0xa0) becomes 0 first. Bit 1: the ground (Terrain_HeightAt);
// else bit 8: the offset alone; else bit 2: Model_FloorHeight of the parts only, bit 4: of the main node only; each of those plus the offset
// (x87, read again after the call). Else 0.
void __cdecl View_FloorAt(float x, float z, float *out);
}
