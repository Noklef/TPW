// The coaster's nodes (0x38 bytes at coaster+0x9c) and its zone boxes.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x43c000: the +4 vector of node index t of the way to node index+1, into out:
// a*(1 - t) + b*t per component (y adds the other way round; same value but for NaNs).
void __cdecl CoasterNode_Lerp(const u8 *coaster, u32 index, float t, float *out);

// 0x43c080: 1 when x has reached the track length (coaster+0xa8), else 0 (also for a NaN).
u32 __cdecl Coaster_IsPastEnd(const u8 *coaster, float x);

// 0x43e9c0: the value at +0x20 of the first of the first count - 1 boxes (0x2c bytes at
// owner+0x78, count at +0x7c) with +0 <= t <= +4, +8 <= pos[1] <= +0xc, +0x10 <= pos[2] <=
// +0x14 and +0x18 <= pos[3] <= +0x1c (a NaN fails); when none has one, or its value is 0,
// the last box's. The second argument is unused.
// sic: with no boxes, count - 1 wraps and the search reads on until it finds a match.
u32 __cdecl Coaster_FindZone(const u8 *owner, u32 unused, const float *pos, float t);

// 0x440c10 (cdecl; coaster; name unconfirmed): the coaster's nodes sampled from the track (the pieces from [0x790fe4]+0x18, a
// cursor over their node lists: +0x90 -> {count, spline nodes +4, second nodes +8}, 0x58 bytes each). First the size: per
// cursor step (TrackCursor_Point at 1 from the last point) __ftol(1 + 3 * distance / spacing (+0xac)) summed; above the capacity
// (+0x98) the block (+0x9c) is freed and 1.1 times as many 0x38-byte nodes are got (GlobalMem_Alloc zeroed). Then t walks the
// cursor's spline in steps of 0.05 (past 1: the cursor steps on; below 0: back), summing the distances; at the first point and
// each time the sum reaches the spacing a node is written: the point (+4) between the last two by how far into the step the
// spacing fell, the spline nodes (+0x2c, +0x30, back along +0x14 while t < 0) and t (+0x34), and the angles (+0x10..+0x18) of
// the tangent (TrackOrientation_Set; the roll that of the second spline's point turned back by them, Vec3_AngleZ). At each
// new spline node its value (+8, on both lists) is the node index plus the way there over the spacing, and its owner's
// (+0x10) +0xfc gets its integer part (when the owner has no +0x74 link, flag 0x2000 or the link names the cursor's index);
// with the cursor past the last piece the end goes on to the piece after the car at +0x54 (one more node; the track length
// +0xa8 then set). Last: +0xa0 = the largest +0xfc step between pieces plus 1, +0xa4 = the count (one more when the length's
// whole part, rounded up, reaches it), the last node copied after it, Coaster_SetSegmentWindow.
// sic: a first piece of one node and no next piece faults (null +0x90), as does a node with no +0x18 next.
void __cdecl Coaster_BuildNodes(u8 *coaster);

// 0x43ea90: resets a nearest-match search: seven zeros, then FLT_MAX as the best distance
// so far (+0x1c).
void __cdecl CoasterSearch_Reset(u32 *search);
}
