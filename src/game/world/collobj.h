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
}
