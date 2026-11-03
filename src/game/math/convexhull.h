// A convex hull in the x-z plane, grown a point at a time: reimplementation of 0x56bd60.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x56bd60 (thiscall (x, y, z), pops 0xc; name invented, unconfirmed). The hull: +0 its points, 0xc bytes each, +0xc0 a byte for each point (its edge from the point before has the new point outside it), +0xd0 the
// count, +0xd4 how many are outside. First and second point: added, the second unless it ties the first in x and z (a NaN ties). Third: on one side of the line through the two it goes between them, on the other
// after them; on the line (an edge with no dx measured by z) it replaces the second past the far end, the first before the near end (a NaN too). Then: no edge with it outside: nothing. One: it goes in the list
// before that edge's point (memmove). More: the list is turned so that the outside run starts it, and rebuilt as the new point, the run's last point, then the points whose edges were inside.
// The list has no limit (sic): over 16 points it runs on over the bytes and the count, over 32 less its turn the original's frame too; the port's reads go to the same memory, the frame's overrun is not kept.
void __fastcall ConvexHull_AddPoint(u8 *hull, void *, u32 x, u32 y, u32 z);
}
