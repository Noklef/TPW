// The animated texture tables: reimplementation of 0x5906e0 and 0x590730.
//
// Six animations, each up to 256 texture frames (0x8c4b98, 0x400 bytes apart) with a frame count
// (0x8c4b80). The texture cache copies its own tables in (0x58d353, from its +0 and +0x1800) and
// clears the counts first (0x58d387); 0x590750 steps through the frames of those with more than
// one by the clock (0x790acc).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5906e0 (frames: 6 x 256 dwords, counts: 6): both copied in.
void __cdecl AnimTextures_Set(const u32 *from, const u32 *fromCounts);

// 0x590730: the six counts cleared.
void __cdecl AnimTextures_Clear();
}
