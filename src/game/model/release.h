// Blocks an instance (+8 its holder, the holder's +4 its model) gives back when it is torn down
// (the teardown at 0x46400a-0x464013 calls both), and the statics after them: reimplementations of
// 0x452540, 0x468200 and 0x46b3b0-0x46b400.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x468200: for a model with flag 0x8000 (+0x30), frees the +0xc block of the first of its
// +0x36 (u16) 16-byte records at +0x54 whose first byte has bit 8, and clears the pointer.
void __cdecl Instance_FreeRecordBlock(const u8 *instance);

// 0x452540: frees the instance's block at +0xcc and clears the pointer.
void __cdecl Instance_FreeBlockCC(u8 *instance);

// 0x46b3b0: frees the holder's block at +0x18 and clears the pointer.
void __cdecl Instance_FreeHolderBlock(const u8 *instance);

// 0x46b3d0: a + b (the code at 0x522972-0x5d805e calls it).
s32 __cdecl Int_Add(s32 a, s32 b);

// 0x46b3e0 / 0x46b400: static initialisers of two floats nothing reads: 0x7b0468 = 1 and
// 0x7b046c = 1 / 2^23 (as camera's).
void __cdecl Release_InitUnusedOne();
void __cdecl Release_InitUnusedEpsilon();
}
