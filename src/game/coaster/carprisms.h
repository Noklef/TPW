// Two ride cars' shapes tested against each other: each a prism of two end faces placed by a car's position and angles, an edge's quad against the other's edges. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x43a2b0 (cdecl, no arguments): the first shape's points (0x7922dc: +0 pairs, +4 their count, used for both shapes, sic) at the car 0x79223c and the second's (0x792244) at 0x792240, each turned by the
// car's angles (+0x18, +0x14, +0x10, by the sine table) and put at its two positions (+4, +0x3c), go to 0x7922f0 and 0x792248. For each quad of the first (its corners also kept at 0x79238c, 0x792384,
// 0x792388, 0x792380), each edge of the second from a point along each way, against the quad's plane: where the point is on the first edge and its four corner angles come to the full turn (0x6fdf7c) or more, or
// where 0x43aa10 says so of the second edge, 1. Else 0.
u32 __cdecl CarPrisms_Touch();

// 0x43aa10 (cdecl; point, t, way): 1 when point + t * way sees the current quad's corners (0x79238c, 0x792384, 0x792388, 0x792380) at angles that come to the full turn or more (a NaN not); else 0.
u32 __cdecl CarPrisms_InQuad(const float *point, float t, const float *way);
}
