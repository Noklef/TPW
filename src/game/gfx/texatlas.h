// The texture pages' square allocator: reimplementation of 0x58c3c0.
//
// Squares are cut from 256 x 256 pages of dwords (0x40000 bytes, TrackedGlobal_Alloc) for the texture cache's items (its one
// caller is the item at 0x58c670, a 0x702278 object). The free rectangles (x, y, width, height: 16 bytes each, up to 32) are at
// 0x8c4928, their count at 0x8c4b30; the page they are in at 0x8c4b28 and the pages made at 0x8c4b2c.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x58c3c0 (cdecl; out, size; name unconfirmed): a square of size dwords a side. 0x100 or more: a page of its own (the pages made
// counted) into *out, 1. Otherwise the free rectangle wasting least (width * height - size * size, the first of equals; one that
// fits exactly is taken at once: the last moved into its place) gives *out = the page + (y * 256 + x) * 4 and is cut: by the
// square's row (y and height) when as wide as it, else by its column when as tall, else split, the new rectangle (count first)
// the column to the square's right, the old one what is below; 0. None fits: a new page, *out it, the free rectangles the part
// below the square (0, size, 256, 256 - size) and to its right (size, 0, 256 - size, size); 1.
u32 __cdecl TexAtlas_Alloc(u32 *out, s32 size);
}
