// The hand-written run-length sprite blitter in cseg (0x6e73c0-0x6ebfc5, its copy tables 0x6ebfc8 and 0x6ec9c8): 8-bit sprites into
// the 8-bit buffer of the parameters at 0xfaa4d8 (pitch, clip width / height, flags, bits, ghost table, clip left / top at 0xfaa4f8).
// A sprite row is a run of bytes ending in 0: n > 0 copies the n bytes after it, n < 0 skips -n pixels (0x80 skips 128).
#pragma once

#include "draw.h"

extern "C" {

// 0x6e73c0 (cdecl (x, y, sprite), x and y relative to the clip; Sprite_DrawRle). Nothing for a zero width or height. Flag 2 draws
// the rows bottom up (the row step -pitch from y + height - 1), flag 1 the columns right to left (from x + width - 1). Rows past the
// clip are skipped in the data; a sprite partly past the clip's sides is drawn by the clipped loops (with flag 1 also at x = 0), the
// rest by the plain ones. Flag 8 blends each pixel through ghost[source << 8 | dest], else flag 0x10 ghost[dest << 8 | source]. The
// signed compares and the copy loops that test their count after the first byte (a clipped width of 0 or less runs on) are the
// original's (sic).
void __cdecl Sprite_DrawRle(s32 x, s32 y, const RleSprite *sprite);
}
