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
}
