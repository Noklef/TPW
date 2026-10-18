// Transformed vertices packed to fixed point in place: reimplementation of 0x563220 (its one caller, 0x57b6e0, packs a batch's 0x20-byte
// vertices: x, y, z, rhw, colour, specular, u, v).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x563220 (cdecl; name unconfirmed): for each of count vertices, stride bytes apart from verts, the 0x18-byte packed one written over
// the buffer from verts, one field at a time as it reads them (so a stride under 0x18 reads what it wrote): x * 32, y * 16, z * 65535
// (__ftol, truncating; out of range or NaN gives 0), the dword at +0x10 (the colour) copied, and u, v (+0x18, +0x1c) * 16. Nothing for
// count 0.
void __cdecl FixedVerts_Pack(u8 *verts, u32 count, s32 stride);
}
