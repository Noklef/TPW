// The depth sort of a batch's triangles (its caller 0x565590 builds the keys): reimplementation of 0x5653d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5653d0 (cdecl; keys, triangles, lo, hi; name unconfirmed): keys[lo..hi] sorted largest first (unsigned) by a quicksort, the 6-byte
// triangles (three u16 indices) swapped with them: the pivot is the key at (lo + hi) >> 1, the left part recursed into and the right one
// looped on. Indices are signed, the sum wraps.
void __cdecl DepthSort_Triangles(u32 *keys, u8 *triangles, s32 lo, s32 hi);
}
