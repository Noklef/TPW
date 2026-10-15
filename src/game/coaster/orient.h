// Coaster track orientation: reimplementation of 0x43e540 and 0x43e590, which turn a
// direction into the yaw and pitch the track pieces and cars are drawn at.
#pragma once

#include "../../../re/types.h"

struct TrackOrientation {  // 0x20 bytes
    u32 flags;             // bit 0: reversed (turn the angles round)
    float yaw;             // +0x04: about y, atan2(x, z) (pi more when reversed)
    float pitch;           // +0x08: atan2(-y, z) of the direction turned back by the yaw
    float roll;            // +0x0c: set by TrackSpline_Frame
    float yaw2;            // +0x10: copies of the final yaw and pitch
    float pitch2;          // +0x14
    float roll2;           // +0x18: its copy
    float rawYaw;          // +0x1c: the yaw before reversing
};
static_assert(sizeof(TrackOrientation) == 0x20, "TrackOrientation is 0x20 bytes");

extern "C" {
// 0x43e590 (direction by value, unused, out, yaw or null): out's angles for the direction.
// The yaw is *yaw when given, else atan2(x, z). The direction is turned by -yaw about y with
// the game's sine table, and the pitch read off it. When out's flag 0 is set, the yaw gets
// pi more and the pitch becomes pi - pitch, each less 2 pi if then above pi. The copies at
// +0x10, +0x14 and +0x1c are moved as bits, so a signalling NaN yaw passed in stays
// signalling there (and at +0x04 unless reversed).
void __cdecl TrackOrientation_Set(Vec3 direction, u32 unused, TrackOrientation *out, const float *yaw);

// 0x43e540 (unused, direction, unused, out, unused, yaw or null): clears out (+0x1c to
// FLT_MAX, which the yaw then replaces) and sets it from *direction.
void __cdecl TrackOrientation_Init(u32 unused, const Vec3 *direction, u32 unused2, TrackOrientation *out,
                                   u32 unused3, const float *yaw);

// 0x432b50: for each piece of the track (0x790fe4: first piece +0x18, next +0x64), runs
// TrackOrientation_Init on each of its nodes' +0x28 direction (nodes 0x58 bytes at
// piece+0x90 -> {count, nodes}; the unused float node+0xc + piece+0x6c), then clears the
// piece's flag 0x800 (+8) unless 0x1000 is set.
// sic: the orientations go to a local and are dropped, so only the flags change.
void __cdecl Track_RefreshOrientations();

// 0x43d8f0: sets the matrix of object's ride (+0xc into 0x7a4610; +8 -> +4 -> +0x78, matrix
// at +0x10) to the identity turned about z, x and y by angles[3], [2] and [1] (sine table
// lookups), turns v (if any) by the same angles in the same order, and copies the matrix's
// third row to object+0x10.
void __cdecl RideObject_Orient(u8 *object, const float *angles, float *v);
}
