// The renderer object's creation, accessors, entry lists and release: reimplementation of 0x5760a0, 0x5761b0, 0x576260, 0x5763f0,
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

// 0x5761b0 (cdecl; name unconfirmed): 0 without a renderer ([0x87a6d0]). Else its frame buffer (+0x10), when set, freed
// (TrackedGlobal_Free, core/gameheap) and cleared; then, [0x87a6d0] read again, its pool (+0), when set, freed with +8 and +0xc
// cleared (+0 kept, sic), the frame buffer freed again when set, the renderer freed (GameHeap_Free) and [0x87a6d0] cleared.
// Returns 1.
u32 __cdecl Renderer_Destroy();

// 0x5760a0 (cdecl; name unconfirmed): without a renderer at [0x87a6d0], a new one of 0x7c bytes (the game's heap) there: +0 to +0x3c,
// +0x70 and +0x78 cleared, +0x40 = 0x30, +0x74 = -1; its pool of 0x24000 bytes (TrackedGlobal_Alloc, core/gameheap) at +0 and, when
// got, all its 0x4800 nodes put on the free list (+4, the last node first) with +8 = 0x4800 and +0xc = 0. 1 (also when the pool was not
// got, sic). 0 when there was one, or when the renderer's allocation failed (the error (3, 0xb0754000) pushed, EngineErrors_Push).
u32 __cdecl Renderer_Create();

// 0x576310 (thiscall; width, height; name unconfirmed): the frame buffer (+0x10), when set, freed (TrackedGlobal_Free) and cleared; +0x38 =
// width, +0x3c = height; a new zeroed one of width * height * 4 bytes (TrackedGlobal_Alloc; 32-bit, wrapping) at +0x10. Got: the render state
// at +0x48 reset (flags 0x1f31, -1.0 at +0x4c / +0x50, 2.0 at +0x54 / +0x58, +0x5c = [0x87afe8]) and, with fewer than 4 (signed) on the
// state stack (+0x60, count +0x70), pushed there unless already on it; then the count set to 1 (sic). 1. Not got: the error (3, 0xb0754000)
// pushed (EngineErrors_Push), 0.
u32 __fastcall Renderer_MakeFrameBuffer(u8 *renderer, void *, s32 width, s32 height);

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

// 0x576410 (thiscall; object, tag; pops 8; name unconfirmed): with a bit of 0x70 in the tag the object goes into the seven slot lists, slot (tag >> 4), by Pool_AddSorted (its result).
// Otherwise, for a tag with bit 1 and an object whose virtual call 2 returns 2 and call 3 a record, the rectangle packed in the record's word at +0xa8 (x0, x1, y0, y1 from the high byte
// down to the low byte, the y0 byte at bit 8) is clamped to the grid and the object added to every cell in it (x outer, y inner): the AND of 1 and their results. 0 in every other case.
u32 __fastcall Renderer_Add(u8 *renderer, void *, u8 *object, u32 tag);

// 0x56b340 (thiscall): 1 when bit 0 of the first dword is already clear. Else it is cleared and the part
// at +4 (a virtual base: the pointer plus the offset at +4 of its vtable; 0 for none) removed from the
// renderer's tag 0x60 list (Renderer_RemoveFromSlot): 1 if that succeeded, 0 if it failed or there is
// no renderer ([0x87a6d0]).
u32 __fastcall Obj_ReleaseFromRenderer(u8 *object);

// 0x56b2f0 (thiscall; name unconfirmed): 1 when bit 0 of the first dword is already set. Else the part at +4 (a virtual base: the pointer plus the offset at +4 of its vtable; 0 for none)
// is added to the renderer's tag 0x60 list (Renderer_Add): when that succeeds bit 0 is set and 1 returned, else 0, as when there is no renderer ([0x87a6d0]). The inverse of Obj_ReleaseFromRenderer.
u32 __fastcall Obj_AddToRenderer(u8 *object);

// 0x48f5c0 (thiscall; id, pops 4; name unconfirmed): nothing when the word at +0x44 is 0 or the word at +8 already equals the low word of id. Else the part at +0xb4 (a virtual base: its address plus
// the offset at +4 of its vtable) is added to the renderer's tag 0x30 list (Renderer_Add) for an id that is not 0 and removed from it (Renderer_RemoveFromSlot) for 0; the word at +8 becomes the id.
void __fastcall Obj_SetRendererPart30(u8 *object, void *, u32 value);
}
