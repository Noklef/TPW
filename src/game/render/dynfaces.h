// The dynamic face queue of the scene: reimplementation of its two writers 0x53c8d0 and 0x53c9c0. The count of faces is the dword at 0x871ee0 (at most 0x806, past which a
// message is printed (left out) and nothing is added); each face is 0xa0 bytes from 0x820b00, with its tag word (n << 5 | tag & 0x1f) in the table at 0x872168.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x53c8d0 (cdecl; a, b, c, d byte, tag, e, f, g, h byte, i, j): a square face. Nothing is added for the tag 1 while the flag byte at 0x763c98 is 0. The face has a, b, c at
// +0, +4, +8, d at +0xc, e, f, g at +0x10, +0x14, +0x18, h at +0x1c, j at +0x90, the dword 1 at +0x94, i (1 while the dword at 0x874fc4 is not 0) at +0x98 and the tag byte at +0x9c.
void __cdecl DynFaces_AddSquare(u32 a, u32 b, u32 c, u8 d, u32 tag, u32 e, u32 f, u32 g, u8 h, u32 i, u32 j);

// 0x53c9c0 (cdecl; corners (4 x 3 dwords), tag, uvs (4 x 2 dwords, stride 12), colours (4 x 3 dwords), user): a polygon face of four corners: the corners at +0..+0x2b, the
// uvs at +0x30 (stride 12), the colours at +0x60 (stride 12), user at +0x90, 0 at +0x94 and +0x98 and the tag byte at +0x9c.
void __cdecl DynFaces_AddPoly(const u32 *corners, u32 tag, const u32 *uvs, const u32 *colours, u32 user);

// 0x53c1f0 (cdecl; tags, count; name unconfirmed): the count tag words at tags[1..count] (the caller hands 0x872168 - 4) sorted by their tag (the low five
// bits): a quicksort with a stack of 50 dwords and insertion sorts of runs under 7, unsigned throughout (a count of 0 goes to the partition). Sic: the
// insertion sort steps down to tags[1], not to the run's start.
void __cdecl DynFaces_SortTags(u32 *tags, u32 count);

}  // extern "C"
