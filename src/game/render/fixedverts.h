// Transformed vertices packed to fixed point in place: reimplementation of 0x563130 and 0x563220 (0x563220's one caller, 0x57b6e0, packs
// a batch's 0x20-byte vertices: x, y, z, rhw, colour, specular, u, v).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x563220 (cdecl; name unconfirmed): for each of count vertices, stride bytes apart from verts, the 0x18-byte packed one written over
// the buffer from verts, one field at a time as it reads them (so a stride under 0x18 reads what it wrote): x * 32, y * 16, z * 65535
// (__ftol, truncating; out of range or NaN gives 0), the dword at +0x10 (the colour) copied, and u, v (+0x18, +0x1c) * 16. Nothing for
// count 0.
void __cdecl FixedVerts_Pack(u8 *verts, u32 count, s32 stride);

// 0x563130 (cdecl; name unconfirmed; three callers, the mesh drawing at 0x5656eb among them): the same packing, but x and y times 8 when the
// flags 0x8bd508 have both 0x80000000 and 0x20, else 16, and z times 32768; each product made a float (stored over the count's argument,
// sic) and rounded to the nearest (fistp: 0x80000000 out of range or NaN) through the dword 0xfb34a0 (x, y) or 0xfb34a4 (z, u, v), which
// keeps the last.
void __cdecl FixedVerts_PackRounded(u8 *verts, u32 count, s32 stride);
}
