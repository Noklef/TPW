// A face's texture entries: reimplementation of 0x46a550.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x46a550: unless the face's flags (+0) have 0x200, sets (set non-zero) or clears bit 0x800
// of the first word of the texture entry each of its +0x5a (u16) 16-byte entries at +0x6c
// points to (set: an OR of the second byte; clear: an AND of the word). The first argument is
// unused.
void __cdecl Face_MarkTextures(u32 unused, const u8 *face, u32 set);
}
