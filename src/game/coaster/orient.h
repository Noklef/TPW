// Coaster track orientation: reimplementation of 0x43e540 and 0x43e590, which turn a
// direction into the yaw and pitch the track pieces and cars are drawn at.
#pragma once

#include "../../../re/types.h"

struct TrackOrientation {  // 0x20 bytes
    u32 flags;             // bit 0: reversed (turn the angles round)
    float yaw;             // +0x04: about y, atan2(x, z) (pi more when reversed)
    float pitch;           // +0x08: atan2(-y, z) of the direction turned back by the yaw
    u32 unknown0C;         // not written
    float yaw2;            // +0x10: copies of the final yaw and pitch
    float pitch2;          // +0x14
    u32 unknown18;         // not written
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
}
