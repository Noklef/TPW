// A polygon clipped to a rectangle on the ground.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x56c5a0 (thiscall (x0, x1, z0, z1), name invented, unconfirmed; 0x56bccb calls it): the polygon's points (12 bytes each from +0, x y z; the count at +0xd0) clipped between z0 and z1, then between x0 and x1.
// Each edge's side at each end is how many of the limits its coordinate is above (a NaN: neither); where the edge crosses a limit (the higher first when it comes down, as the sides' table has it), a point by Segment_AtZ or
// Segment_AtX (the edge's end, then its start); a start between them is kept. The count is read back for each edge. Sic: the z pass's 16 points are not checked: past them the original runs on over its frame, this over its own.
void __fastcall Polygon_ClipToRect(u8 *polygon, void *, float x0, float x1, float z0, float z1);
}
