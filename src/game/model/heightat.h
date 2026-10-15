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
}
