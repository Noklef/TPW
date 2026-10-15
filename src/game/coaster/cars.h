// The coaster's cars along its track. A car is 0x80 bytes, in an array at coaster+0x128
// (count at +0xf4); its position along the track (+0x30) is a fractional coaster node index.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x43c160: how fast the car swings back, from its angle at +0x94: zero when the angle is
// zero (or NaN) or flag 0x100 (+8) is set; else (pi - angle, or -pi - angle when it is
// negative) * x / type+0x10 / (d1 + d2), with the coaster's type (+4), and d1 and d2 the
// distances (between the +0x4c positions) to the cars at +0x68 and +0x64. Left in st0, so
// double. (d1 is kept, as a float, in the car argument's stack slot.)
double __cdecl CoasterCar_SwingRate(const u8 *coaster, const u8 *car, float x);

// 0x43c1f0: spaces the cars along the track. Each car first takes the float at
// car+0x28 + (car+8 + car+0xc) * 0x60 - 0x20 (its spline node's predecessor's +0x40), plus
// the track length (coaster+0xa8) when negative (or NaN). Then from the second car round
// to the first, each is put one gap (type+0x2ec / 1000) behind the one before: from that
// car's position p, walk the coaster nodes (0x38 bytes at +0x9c, count +0xa4; length scale
// coaster+0xac over node+0x1c each, the first one only for 1 - frac(p)) until the sum r
// reaches the gap, then interpolate between the last two nodes: (n-1)*(1-w) + n*w with
// w = (r - gap) / (r - previous r), wrapped by fmod(length + x, length). Stores the first
// car at coaster+0x124.
// sic: no cars divides by zero, and a NaN or a node length that is zero or negative never
// reaches the gap, so the walk loops forever; node indices are not checked.
void __cdecl Coaster_SpaceCars(u8 *coaster);
}
