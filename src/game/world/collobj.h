// The track rides' collision objects (the module that reports "Out of Collision Objects"):
// reimplementation of the pool reset 0x544330, the list releases 0x5457e0 / 0x545810 and the chunk list release 0x54b290. The
// pool [0x877b88] holds 0x400 objects of 0x14 bytes, free ones chained by +0x10 from the head
// [0x877b8c]. A collider's lists chain their nodes by +0x14; bit 0 of a node's +0 marks it in
// use; [0x877b74] and [0x877b7c] count the nodes in use of the two kinds.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x544330: every pool object chained to the next by +0x10 (the last to 0); the free head the
// first.
void __cdecl CollPool_Reset();

// 0x5457e0 (collider): each node of the list at +0xc0 cleared (+0 = 0) and taken off the count
// [0x877b74]; the list itself is left as it is.
void __cdecl Collider_ReleaseC0(u8 *collider);

// 0x545810 (collider, mask): the list at +0xa8 cut to its nodes with any of mask's bits in +0, in
// order; each other node taken off the collider's count (+0xac) and [0x877b7c] and its bit 0
// cleared; every node's link (+0x14) cleared before it is relinked.
void __cdecl Collider_KeepA8(u8 *collider, u32 mask);

// 0x54b290 (cdecl; owner): every chunk on the list at owner +0xbc is freed: its nodes (the list at
// chunk +0x1c, next at node +0x14) have bit 0 of their first dword cleared and 0x877b74 stepped down,
// then the owner's head = chunk +0x24, bit 0 of the chunk's first dword cleared and 0x877b84 stepped down.
void __cdecl Pool_FreeAll(u8 *owner);

// 0x549070 (cdecl; area, x, y, self; name unconfirmed; the track rides', 0x547755 and 0x547ec5): of the area nodes in use (bit 0; the
// table [0x877b70] walked until [0x877b74] of them are seen, so a count above the nodes in use runs off its end) the one nearest (x, y)
// (IntVec2_Length of its centre minus the point, strictly less than the best so far; the first found on a tie) that is not a blocker
// (flag 0x40), is not area itself unless self, meets area (IntArea_Overlaps(area, node)) and whose centre moved 0x87c towards its closed
// side (y - 0x87c without flag 8, else + 0x87c without 0x10; x + 0x87c without flag 2, else - 0x87c without 4) lies in no blocker
// (IntArea_Contains, the table and count reread for each). 0 for none, no table or a count of 0.
u8 *__cdecl CollArea_FindNearest(const s32 *area, s32 x, s32 y, u32 self);

// 0x547c60 (cdecl; car, x, y; name unconfirmed; the track rides' 0x547815, 0x547927 and 0x5479b2): a ride car steered towards (x, y); returns
// the distance left, or -1. With flag 0x10000 (+0) and an area (+0x98): the first area node in use (the table [0x877b70] walked until
// [0x877b74] are seen) that is no blocker (0x40) and contains the point (IntArea_Contains); none: the target (+0x6c, +0x70) is the point and
// -1 returned. When the car's area meets that node (IntArea_Overlaps) the target is the point plus the offset (+0x74, +0x78) and the flag
// cleared; else the nearest node (CollArea_FindNearest(area, x, y, 0); none: the velocity (+0x3c, +0x40) zeroed and -1, after the empty
// Debug_Printf) with its centre moved 0x87c towards its closed side as CollArea_FindNearest does is the target, flag 0x20000 set and
// 0x10000 cleared. Then: the bearing to the target, atan2(dx, -dy) / pi * 256 - 256 truncated (fpatan, x87 at the game's precision); the
// turn (-heading (+0x50) - bearing) & 0x1ff; the distance sqrt(dx^2 + dy^2) truncated, both halved (arithmetically) until |dx| <= 0x8000
// and |dy| <= 0x7fff and the result shifted back. The kind (+0x9c): a turn past 0x100 is taken the other way (0x200 - turn, the rate
// +0x28 negated); a turn beyond rate / 2 with the kind's +0x54 not 2 and flag 0x4000 turns the heading by the rate (& 0x1ff). A turn below
// distance / 512 kept to 0x16-0x30, with flags 0x84000 just 0x4000 and +0x54 not 2, adds speed (+0x20 + the car's +0x68; 3/4 of it unless
// the area's flags have all of 0x1e) * the integer sine (0x877358) of the heading * 4 / 1024 to +0x3c, and of the heading + 0x80 to +0x40.
s32 __cdecl Collider_SteerTo(u8 *car, s32 x, s32 y);
}
