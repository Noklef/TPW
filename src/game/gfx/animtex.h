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

// 0x590750 (name invented, unconfirmed): while the flag byte 0x8c639c is on, from the height -4 * the clock (0x790acc) down to the screen's (0x7a1930), a row for each animation
// (of over one frame) of its frames in the renderer's slot 10 (x, y, x + w, y + h, -1, texture), each 2 past the last's size, sliding by the count 0x8c63a0 * 0x102 and the screen's
// width (0x7a1968) a pass; mode 4 (0x8bd524) leaves out the ones not inside 0x8bcbd0, 0x8bcbd4. Sic: the first animation at one frame or less leaves the row's height a stack slot not yet written.
void __cdecl AnimTextures_DrawSheet();
}
