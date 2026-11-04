// A ride's animation stepped, alone or all of them: reimplementation of 0x44e380 and 0x44e410. Names invented.
#pragma once

#include "../../../re/types.h"

struct AddrCall;
void RideAnimate_SetCalls(const AddrCall *table, u32 count);  // (the comparison driver's seam)

extern "C" {

// 0x44e380: with `shown`, a ride marked 0x800000 has its layers reset (unless 0x80000) and the mark cleared. A ride with an animator
// (+0x14) and neither 0x3000 flag updates it; when shown, its aim sprites too (if its part +8 has +0x28). 0x20: its strip animation steps.
void __cdecl Ride_AnimateOne(u8 *ride, u32 shown);

// 0x44e410: mode 2: the followed ride (0x790988), if 0x790ab0 has 4, updated as shown. Else every ride in the table 0x7a4614 (counts
// reread each turn): 0x200000 is cleared and nothing more; else updated as not shown, stepped and marked 0x800000.
void __cdecl Rides_AnimateAll(u32 mode);
}
