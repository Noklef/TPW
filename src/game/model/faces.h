// A face's texture entries and flags: reimplementation of 0x469930 and 0x46a550.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x469930: 1 when flags has any of its low 4 bits (as faces' and cells' flags are
// tested), else 0.
u32 __cdecl Flags_AnyLow4(u32 flags);

// 0x46a550: unless the face's flags (+0) have 0x200, sets (set non-zero) or clears bit 0x800
// of the first word of the texture entry each of its +0x5a (u16) 16-byte entries at +0x6c
// points to (set: an OR of the second byte; clear: an AND of the word). The first argument is
// unused.
void __cdecl Face_MarkTextures(u32 unused, const u8 *face, u32 set);
}
