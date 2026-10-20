// A ride base's texture coordinates tidied as its model is set up: reimplementation of 0x467310.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x467310 (cdecl (model, part, tolerance, map, positions, warned); name unconfirmed): the part's texture coordinates (+0x68, u[4] v[4]
// per 0x20 bytes, +0x5e of them, u16) tidied. First each u and v whose fraction (CRT modf) lies within the tolerance of a whole number
// is snapped onto it: below -(1 - tolerance) (NaN too) to the whole part - 1, between -tolerance and tolerance (exclusive) to it, above
// 1 - tolerance to it + 1. Then each triangle (+0x70, 8 bytes, vertex indices u16 at +2, +4, +6; +0x5c of them) takes the least u and
// v of its corners (NaN-sticky as the x87 compares them), and the vertices are moved, each once (by the first triangle naming it),
// by the floor of its triangle's least u and v. Last, per group (+0x6c, 0x10 bytes: +0 a material whose first word holds flags, +4
// the first triangle, +6 the count; +0x5a of them): with a map and positions (12-byte points through the part's u16 map at +0x94),
// a triangle's corners count as off the base unless its centroid's y lies in (-0.1, 0.1) and the map's byte at
// __ftol(x * 0.1) * 16 - __ftol(z * -0.1) is set; a group with none off has its material's bits 4 and 5 cleared (unless its bit 15 is
// set). Any vertex (on the base, or all without a map) left outside 0..1 sets *warned once (the Debug_Printf naming the model, +0x18,
// is left out). The three scratch blocks are GlobalMem_Alloc'd (zeroed) and freed.
void __cdecl RideBase_TidyUVs(const u8 *model, u8 *part, float tolerance, const u8 *map, const float *positions, u32 *warned);
}
