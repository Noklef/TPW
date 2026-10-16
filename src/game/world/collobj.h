// The track rides' collision objects (the module that reports "Out of Collision Objects"):
// reimplementation of the pool reset 0x544330 and the list releases 0x5457e0 / 0x545810. The
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
}
