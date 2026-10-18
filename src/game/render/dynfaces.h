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

// 0x53cab0 (cdecl; dx, dy, dz, rows, cols, row, col; its callers push two more, 0x10 and 0x13, unread; name unconfirmed): the quads of a
// grid 19 vertices wide (positions at 0x874180, colours at 0x870ed8, 12 bytes each, both as 0x53d570 fills them) queued as polygon faces
// (DynFaces_AddPoly, user 1): for each column i from col (cols of them) and each row from row (rows of them), the corners are the vertex
// and those below, right and below-right of it, offset by (dx, dy, dz) on the x87, with their colours (through the x87: a signalling NaN
// comes out quiet), the uvs (0, 0) (1, 0) (0, 1) (1, 1) in that order, and the tag the next of the dwords at 0x871d18.
void __cdecl DynFaces_AddGrid(float dx, float dy, float dz, s32 rows, s32 cols, s32 row, s32 col);

// 0x53d380 (cdecl; rows, cols, step, phase; name unconfirmed): that grid filled as a wave, x87 at the game's precision with the sine table
// (sintable.h): with k = 2 pi / 3 and A = 10 degrees, for each column (z = 0, step, ...) and row (x = 0, step, ...) the vertex is
// (x, 1.7 cos h, z), h = x * k cos A - z * k sin A - phase / 2, and its normal (sin h, 1, sin h) normalised.
void __cdecl DynFaces_WaveGrid(s32 rows, s32 cols, float step, float phase);

// 0x53c1f0 (cdecl; tags, count; name unconfirmed): the count tag words at tags[1..count] (the caller hands 0x872168 - 4) sorted by their tag (the low five
// bits): a quicksort with a stack of 50 dwords and insertion sorts of runs under 7, unsigned throughout (a count of 0 goes to the partition). Sic: the
// insertion sort steps down to tags[1], not to the run's start.
void __cdecl DynFaces_SortTags(u32 *tags, u32 count);

// 0x53c810 (cdecl; name unconfirmed): with the flag at 0x874fcc set, each of the three objects the set-up at 0x53bfe0 makes (the pointers at 0x871f64,
// 0x871ee4, 0x871ed8, in that order; null ones skipped, none cleared, sic) has its animator at +0x54 put to sleep (Animator_Sleep) and disposed of
// (Animator_Dispose) and is freed (Mem_Free); then bit 0 of the dword at 0x763c78 is cleared (Dword_ClearBit0). The flag is cleared either way.
void __cdecl DynFaces_ReleaseModels();

}  // extern "C"
