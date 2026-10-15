// Id tables: reimplementation of 0x468760-0x4688a6. A table of pointers indexed by id from 1
// (the coaster links' is 0x790bd0, 0xff ids), with three counters kept by the caller: used,
// holes (free ids below the highest used) and next (the id the next add takes).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x468760: stores value under a new id (or under id, unless that is -1, returning it).
// Returns 0, storing nothing, when there are no holes and used has reached capacity. Taking a
// hole moves next to the first free id after it, scanning up to holes + used (next stays put
// when the scan finds none, sic).
u32 __cdecl IdTable_Add(u32 *used, u32 *holes, u32 *next, u32 capacity, u32 *table, u32 value, s32 id);

// 0x468810: frees id (nothing for 0): clears its entry and fixes the counters, dropping the
// holes the freed id leaves at the top.
void __cdecl IdTable_Remove(u32 *used, u32 *holes, u32 *next, u32 *table, u32 id);
}
