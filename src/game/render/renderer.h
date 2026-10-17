// The renderer object's accessors and its entry lists: reimplementation of 0x576260, 0x5763f0,
// 0x576400, 0x576540, 0x5765c0, 0x5766e0 and 0x576820.
//
// The renderer is the object at [0x87a6d0] (built at 0x576162 with a pool of 0x4800 8-byte nodes
// at +0; its frame buffer, +0x38 wide by +0x3c high, at +0x10). +0x48 starts its render state
// (the flags, 0x1f31 when the buffer is made, 0x576379), which callers save, replace (0x21,
// 0x2821) and put back around their drawing (0x4a3aa0, 0x5f0bad). +0x78 is a function called
// when the display device has been restored since the last frame (its generation, 0x8bcc40,
// differs from +0x74; 0x576c26).
//
// Entries are 8-byte nodes {key, next} drawn from the pool: +4 heads the free list and +0xc counts
// the live nodes. Live nodes hang off list heads: a grid of +0x38 by +0x3c heads at the pointer
// in +0x10, and seven slot heads at +0x14-+0x2c, slot n chosen by (tag >> 4) for a tag with a
// bit of 0x70. Removing a node returns it to the free list. A removal that finds nothing sets
// 0x80000000 in the scene flags (0x8bd508), and so does every list cleared by 0x576820 and
// 0x5765c0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x576260 (thiscall; fn): +0x78 = fn, the device restore callback (0x54e2b0, or 0).
void __fastcall Renderer_SetRestoreCallback(u8 *renderer, void *, u32 fn);

// 0x5763f0 (thiscall; flags): +0x48 = flags.
void __fastcall Renderer_SetStateFlags(u8 *renderer, void *, u32 flags);

// 0x576400 (thiscall): +0x48.
u32 __fastcall Renderer_StateFlags(const u8 *renderer);

// 0x576540 (thiscall; key, tag): the first node with that key leaves the tag's slot list.
// 0 without a bit of 0x70 in the tag; otherwise 1 if found, or 0 with the scene flag set.
u32 __fastcall Renderer_RemoveFromSlot(u8 *renderer, void *, u32 key, u32 tag);

// 0x5765c0 (thiscall; tag): every node leaves the tag's slot list, which is set empty, and the
// scene flag set. 0 without a bit of 0x70 in the tag, else 1.
u32 __fastcall Renderer_ClearSlot(u8 *renderer, void *, u32 tag);

// 0x5766e0 (thiscall; x0, y0, x1, y1, key): removes the node with that key from each grid cell
// of the rectangle (clamped to the grid; the order is rows, then columns). 1 when every cell
// had one (and when the grid is missing or the rectangle empty after clamping), else 0 with
// the scene flag set; 0 as well, doing nothing, for a rectangle wholly outside the grid.
u32 __fastcall Renderer_RemoveInRect(u8 *renderer, void *, s32 x0, s32 y0, s32 x1, s32 y1, u32 key);

// 0x576820 (thiscall): empties every grid cell (the scene flag set for each) and the seven slot
// lists, returning their nodes to the free list; always 1. The grid pointer is not checked (sic).
u32 __fastcall Renderer_ClearAll(u8 *renderer);

// 0x576620 (thiscall on the grid pool {+4 free nodes, +0xc count, +0x10 the cells (heads, 4 bytes each), +0x38 width, +0x3c height}; x0, y0, x1, y1, object, an unused word, pops 24): 0 when
// the rectangle is entirely outside (past the width / height, or ending below 0); otherwise it is clamped to the grid and Pool_AddSorted is run for the object on every cell in
// it (x outer, y inner); the result is the AND of 1 and their results.
u32 __fastcall Grid_AddObject(u8 *grid, void *, s32 x0, s32 y0, s32 x1, s32 y1, u8 *object, u32 unused);

// 0x5ec220 (thiscall destructor-like): slot 6 (offset 0x18) of the object at +0x14 is called; then for the groups at +0x3c4 and +0x358 the four arrays at +0x2c, +0x3c, +0x4c and
// +0x5c are Array3D_Free'd, the last one's block is freed and cleared and BufferC_FreeAndClear runs on +0x4c, +0x3c and +0x2c; then the blocks at +0x28 and +0x18 are freed
// when set and their four dwords cleared.
void __fastcall Renderer_Teardown(u8 *object);
}
