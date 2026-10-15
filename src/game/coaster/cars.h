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

// 0x43bc20: sets each of the car's segments (0x60 bytes at car+0x28, count +0xc) at its
// distance from the middle one (index car+0x2c): +0x38 the running sum of the segment
// lengths from the middle (exclusive) outwards, positive towards index 0 and negative
// past it, and +0x3c that over coaster+0xac; the middle one gets zeros. A segment's
// length is the car's type (+4) +0x1c when its +8 is 0, +0x24 when it is count - 1, else
// +0x20. The sums stay unrounded (only the stores round).
// sic: the middle index is not checked against the count.
void __cdecl CoasterCar_SetSegmentOffsets(const u8 *coaster, u8 *car);

// 0x43b750: the coaster's segment window (+0xf8, +0xfc): with a the float at
// car->0x90->4->8 for the car at coaster+0x54 and b the same for the car two along it (+0x64
// twice), and k the coaster type's (+4) +0x31c, each end is k of the way from its value to
// their midpoint: k * (a + b) / 2 + (1 - k) * a, and the same with b.
void __cdecl Coaster_SetSegmentWindow(u8 *coaster);

// 0x43b7e0: of the car's segments (index car+8 on, car+0xc of them), the first and the last
// whose span, from +0x40 - type+0x28 to +0x40 + type+0x24 (type at segment+4, both over
// coaster+0xac), reaches into the coaster's segment window (front >= +0xf8 and back not
// past +0xfc; a NaN front fails, a NaN back passes). Their indices into car+0x28 go to
// *first and *last.
// sic: with no match the index is that of a null pointer, -(car+0x28) / 0x60.
void __cdecl CoasterCar_FindSegmentsInWindow(const u8 *coaster, const u8 *car, s32 *first, s32 *last);
}
