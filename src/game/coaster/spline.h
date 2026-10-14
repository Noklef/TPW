// Coaster track splines: reimplementation of 0x43e1b0 and 0x43e320, the cubic Bezier
// between two track nodes and its derivative, as the track builder and the test run use them.
//
// A node holds its position at +0x40, the control point ahead of it at +0x4c and the one
// behind it at +0x34 (each a Vec3), so the segment from node `from` to node `to` is the
// Bezier of from.position, from.ahead, to.behind and to.position.
// Each component keeps the original's expression tree (the compiler associated x, y and z
// differently) in x87 arithmetic (see x87.h), so results match bit for bit.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x43e1b0 (from, to, t, out): the point at t (0..1).
void __cdecl TrackSpline_Point(const u8 *from, const u8 *to, float t, Vec3 *out);

// 0x43e320 (from, to, t, out): the derivative at t, d point / dt (not normalised).
void __cdecl TrackSpline_Tangent(const u8 *from, const u8 *to, float t, Vec3 *out);
}
