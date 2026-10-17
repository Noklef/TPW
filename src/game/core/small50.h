// A grid of object lists and the 16-bit colour tables of a surface: reimplementation of 0x576620 and 0x5830e0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x576620 (thiscall on the grid pool {+4 free nodes, +0xc count, +0x10 the cells (heads, 4 bytes each), +0x38 width, +0x3c height}; x0, y0, x1, y1, object, pops 20): 0 when
// the rectangle is entirely outside (past the width / height, or ending below 0); otherwise it is clamped to the grid and Pool_AddSorted is run for the object on every cell in
// it (x outer, y inner); the result is the AND of 1 and their results.
u32 __fastcall Grid_AddObject(u8 *grid, void *, s32 x0, s32 y0, s32 x1, s32 y1, u8 *object);

// 0x5830e0 (thiscall; twelve words, pops 48): the words go to +0x80 .. +0xac (three groups of three then another three); when the dword at +0x1c is 0x10 the two tables of 256
// dwords at +0xb0 and +0x4b0 are made from them (shift counts of the low 5 bits).
void __fastcall Surface_SetMasks(u8 *surface, void *, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j, u32 k, u32 l);

// 0x5e2a80 (thiscall on the list {+4 first, +8 last, +0xc count} of {value, next at +4, previous at +8} nodes; value, pointer to the iterator, pops 8): a new node (zeroed,
// 12 bytes) holding the value goes before the node the iterator is (at the end for none; at the front for the first node) and the count grows.
void __fastcall PtrList_Insert(u8 *list, void *, u32 value, u8 **position);

}  // extern "C"
