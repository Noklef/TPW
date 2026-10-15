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
#include "orient.h"

extern "C" {
// 0x43e1b0 (from, to, t, out): the point at t (0..1).
void __cdecl TrackSpline_Point(const u8 *from, const u8 *to, float t, Vec3 *out);

// 0x43e320 (from, to, t, out): the derivative at t, d point / dt (not normalised).
void __cdecl TrackSpline_Tangent(const u8 *from, const u8 *to, float t, Vec3 *out);

// 0x43e6b0 (nodes, flags, from, to, upFrom, upTo, t, out, orient or null): the point at t
// into out and, given orient, the frame there: yaw and pitch along the tangent
// (TrackOrientation_Set, passed a blend of the nodes' +0xc plus their piece's +0x6c, which
// it ignores), and the roll of the matching point on the upFrom-upTo spline about the
// tangent (atan2 of its offset turned back by the yaw and the pitch). To the roll is added
// a blend from upFrom to upTo by t of
// - flags bit 3 set: the coaster nodes' banks (0x38-byte nodes, bank at +0x18), node
//   number (int)up[+8] (__ftol, so an index out of range reads out of the array);
// - else: their pieces' (node +0x10) +0xe4, weighted by the blend's fraction: with k(up)
//   = up's piece's first dword (unsigned) + up[+0xc], the blend of the k minus the whole
//   part of k(upFrom), clamped to 0..1 (NaN as 0).
// The roll goes to orient +0x0c and +0x18; the double from atan2 is added unrounded in the
// first case, the stored float in the second.
void __cdecl TrackSpline_Frame(const u8 *nodes, const u8 *flags, const u8 *from, const u8 *to, const u8 *upFrom,
                               const u8 *upTo, float t, Vec3 *out, TrackOrientation *orient);
}
