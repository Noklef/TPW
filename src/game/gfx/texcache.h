// The texture cache's items (the cache's log is c:\tcache.log; set up by 0x58da40, "Texture
// Cache Initialization"): the abstract base with vtable 0x702198 that its surfaces derive from
// virtually (0x58b3e0 builds one at +0x3c of a 0x702218 object). Reimplementation of the
// base's constructor and flag word methods 0x58b300-0x58b390 (thiscall), and an item's key (0x58cb40).
//
// +4 is a flag word: bits 1, 2 and 4 are tested by the cache (0x58cd07, 0x58fe3d, 0x59007f);
// 0x10000000 survives SetFlags.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x58b890 (thiscall on the cache; dest, source): a 2 x 2 box filter, the next mip level. The
// surfaces have height +8, width +0xc, pitch +0x10 and pixels +0x24; the dest's width and
// height give the pixels written, the source being twice as big. The cache's +0x30 is the
// pixel format: 3 = 32-bit (each channel the rounded average of four: (sum + 2) >> 2 for the
// bytes 0 and 2, the same through a >> 2 for bytes 1 and 3, whose rounding is 0x80),
// 2 = 16-bit 4-4-4-4 (the same on the nibbles, rounding 2 and 0x20); others do nothing.
// Rows then count down from the height and pixels from the width, so zero does nothing here.
void __fastcall TexCache_Halve(u8 *cache, void *, u8 *dest, const u8 *source);

// 0x58eae0 (stdcall; out, listB, owner): merges two lists of cache items (next at +4, flags +8,
// position s16 +0x10 (x) and +0x12 (y), size s16 +0x14 by +0x16) into the array out, zero-ended,
// and returns the sum of width * height of what was put there. The owner's list (+4) is
// skipped past its first item if that uses the driver filter (0x590fb0), then only items with
// flag 0x40000000 count; listB's items count unless they use the driver filter. While both have
// items the one with the greater y goes first (then the greater x), listB's winning ties only
// when strictly greater: equal positions take the owner's. What remains of listB goes next,
// then what remains of the owner's. Maxima of width and height are tracked but never used.
u32 __stdcall TexCache_MergeItems(u32 *out, const u8 *listB, const u8 *owner);

// 0x58eca0 (thiscall on the cache; items, out, outCount, area, a fifth never read; pops 0x14; name unconfirmed; 0x58f540 and 0x58f639 ask it):
// a zero-ended array of items packed into the area's s32 width (+8) and height (+0xc), out getting 0x14-byte rows (x, y, width, height,
// item), zero-ended at the row's +0x10, and *outCount their number. Free rectangles start as the whole area and stay sorted (RecordSort_Run
// on a key of width << 16 + height, from the rectangle used up); each item (s16 sizes +0x14 / +0x16, rounded up by the mask +0x18e4 when
// +0x18e8 is set) takes the last that holds it. With cache flag 2 (+0x18cc) the row is rounded up and the item centred in it. What is left
// of the rectangle is split by leftovers over 8 on each side. An item with no room while nothing is placed yet, cache flag 1 set, flags & 0x30
// (with the driver filter, also a width above [0x702434]), is halved (flags - 0x10, sizes (n + 1) >> 1, its key set) and moved down the array
// by descending s32 +0x38, and the new one at its place tried; other items with no room clear the answer. Stops at 0x7f rows or no
// rectangles. +0x1918 / +0x191c get the widest and highest; +0x1924 is set by a halving. 0x8c4b6c counts the rows, 0x8c4b68 likewise but is not cleared.
// Returns 1 when every item found a place.
u32 __fastcall TexCache_PackItems(u8 *cache, void *, u8 **items, u8 *out, u32 *outCount, const s32 *area, u32 unused);

// 0x58d0f0 (thiscall on the cache, 0x1930+ bytes): its default settings: five 0x80s at
// +0x18b0-+0x18c0, a falling series of floats (0.5, 0.25, 0.17, 0.065, 0.012, 0.0030) at
// +0x187c-+0x1890, +0x1878 = 4, +0x18ac = 0x100, +0x18e4 = 7, +0x1900 = 1.0f, the entry
// pointers +0x1800-+0x1814 and +0x1830-+0x1844 empty, and a dozen more fields zeroed. Returns the cache.
u8 *__fastcall TexCache_InitDefaults(u8 *cache);

// 0x590fd0 (thiscall): the cache's 0x20-byte entry (made at 0x58d2b0): +0, +4, +8 and +0xc zeroed,
// +0x14 = -1; the rest left as it was. Returns the entry.
u8 *__fastcall TexCacheEntry_Ctor(u8 *entry);

// 0x58b300: vtable 0x702198, flags 0; returns item.
u8 *__fastcall TexCacheItem_Ctor(u8 *item);

// 0x58b330 (flags): the flag word = flags | its bit 0x10000000.
void __fastcall TexCacheItem_SetFlags(u8 *item, void *, u32 flags);

// 0x58b350 / 0x58b360 / 0x58b370: the flag word & 1 / & 2 / & 4.
u32 __fastcall TexCacheItem_Flag1(const u8 *item);
u32 __fastcall TexCacheItem_Flag2(const u8 *item);
u32 __fastcall TexCacheItem_Flag4(const u8 *item);

// 0x58b380 / 0x58b390: flag 1 set / cleared.
void __fastcall TexCacheItem_SetFlag1(u8 *item);
void __fastcall TexCacheItem_ClearFlag1(u8 *item);

// 0x58dc50 (thiscall on the cache; item; pops 4; name unconfirmed): without the cache's flag 0x20000000 (+0x18cc) 1. A null item 0, one
// without flag 0x1000 (+8) 1. Otherwise the item is looked for in its owner's (+0x18) list (+4, next at +4): not there 0; found, it is
// unlinked, its flags lose 0xf030 and get their bits 0x40 and 0x80 (shifted right by 2, arithmetically) as 0x10 and 0x20, +0x14 and
// +0x16 become the words at +0x10 and +0x12 rounded up to multiples of 8, and +0x38 a key: ((__ftol((1.5 - the float at +0x30) * 2047)
// & 0xfff) << 8 plus the s16 width, << 8, plus the s16 height, with 0x20000000 for flag 0x4000 (never: just cleared, sic) and
// 0x40000000 (set in memory) for flag 0x80000. Returns 1.
u32 __fastcall TexCache_UnlinkItem(u8 *cache, void *, u8 *item);

// 0x58cb40 (thiscall on the item; name unconfirmed): +0x38 = the key as TexCache_UnlinkItem's, from the words at +0x14 and +0x16 as they
// are: ((__ftol((1.5 - the float at +0x30) * 2047) & 0xfff) << 8 plus the s16 at +0x14, << 8, plus the s16 at +0x16, with 0x20000000
// for flag 0x4000 (+8) and 0x40000000 (set in memory) for flag 0x80000.
void __fastcall TexCacheItem_SetKey(u8 *item);

// 0x58faf0 (thiscall on the cache; name unconfirmed): for each of the six entry sizes (the sides at 0x702420: 0x100 down to 8, the
// area the side squared), the entries at +0x400 * size (the signed count at +0x1800 + 4 * size) with items (+4) whose s32 +0x1c is
// over 0 and its << 2 (s32) under the area and whose key (s32 +0xc) * 2 under it too are sorted by key, ascending, keeping
// min(s32 +0x1878, 16) of them: each is inserted after the keys not above its own, those past it moved up one by a forward copy, so
// what was at its place fills the rest (sic), and the last falls off when full. A 17th with 16 kept writes one past both arrays (sic):
// keys[16] is entries[0] (the frame is modelled), and entries[16] is the return address, so the function ends by jumping to that entry
// pointer (a fault; the eviction may fault first on entries[0], now a key).
// From the last kept down: the entry's items (+4, next at +4) are taken off it (+4 = 0), each with its cache flags cleared
// (Texture_ClearCacheFlags) and +0x1c = 0, and one with flag 0x40000000 (+8) gets 0x2000 (and 0x4000 when it had that) and goes onto
// the cache's list at +0x18c4; then the entry's +0 has its flags set to 0 (TexCacheItem_SetFlags).
void __fastcall TexCache_Evict(u8 *cache);

// 0x58d8e0 (thiscall on the cache; name unconfirmed; called at 0x576af6): the cache's bit 0 (+0x18cc) set when [0x8bd508] has a bit of
// 0xc0000000, else cleared; Table879d08_FillFrom (the slot index 0-15 at 0x879d48). Then for each of the six entry sizes, the entries at
// +0x400 * size (the signed count at +0x1800 + 4 * size, read again each time): each item of the entry's list (+4, next +4) has its
// flag 0x2000 (+8) cleared; through its virtual base (the offset at +4 of the table its +0 points to) the base's byte at the slot index is
// taken off its +0x10 and cleared, and its s32 +0x14 halved (an arithmetic shift); then flag 0x40000000 cleared, the float +0x30 = 1 and,
// with the cache's bit 0, Texture_ClearKeep. With the bit 0 (tested again), the entry's +0 gets TexCacheItem_ClearFlag1. Last the list at
// +0x18c4 has every flag 0x2000 cleared, and +0x18c4 is cleared.
void __fastcall TexCache_BeginFrame(u8 *cache);

// 0x58e8d0 (thiscall on the cache, not used; list head pointer, item, pops 8; name unconfirmed; called at 0x58f416 and 0x58f705): each item of
// the chain from item (next +4) has its cache flags cleared (Texture_ClearCacheFlags) and +0x1c = 0. One with flag 0x40000000 gets flag
// 0x2000 and goes into the list in front of the first whose key (s32 +0x38) is not above its own (the end otherwise). Any other has flags
// 0xf030 cleared and bits 6 and 7 copied to 4 and 5, its size (+0x10, +0x12) rounded up to 8 at +0x14 / +0x16 (16-bit), its key set as
// TexCacheItem_SetKey does (inlined) and its virtual base's frame slot decayed as TexCache_BeginFrame does.
void __fastcall TexCache_RequeueItems(u8 *cache, void *, u8 **list, u8 *item);

// 0x58dd60 (thiscall on the cache; mode, pops 4; name unconfirmed; called from a dozen places, 0x46b6f0 the first): for each of the six
// entry sizes, each entry (the signed count at +0x1800 + 4 * size, re-read per entry) has its item list (+4) taken off. Mode bit 2:
// the entry's +0 gets TexCacheItem_SetFlags(0), then items with flag 0x200000 are reset and dropped, the others pushed back on the
// entry's list (so reversed). Without it every item is reset and dropped. A reset clears flags 0x7000 and 0x30, copies bits 6 and 7
// to 4 and 5, rounds the sizes +0x10 / +0x12 up by the cache's mask (+0x18e4) into +0x14 / +0x16 and sets the key as
// TexCacheItem_SetKey does (inlined without bit 2); mode bit 1 also zeroes the first six dwords of its virtual base.
void __fastcall TexCache_ReleaseItems(u8 *cache, void *, u32 mode);

// 0x58e1d0 (thiscall on the cache; items, count, tier, limit; pops 0x10; name unconfirmed; called at 0x58e529 and 0x58e5a8): the items
// halved while the cache's total area (s32 +0x18dc) stays at or above limit. The tier picks a pair at 0x702360: the least flags & 0xc0
// (signed) and a band, whose floats at +0x1904 and +0x1908 + 4 * band bound the item's float +0x30 (above the first, not above the second).
// An item also needs flags 0x30 (with the driver filter, Texture_UsesDriverFilter, a width (s16 +0x14) above the s32 at 0x70235c, 8)
// and not flag 0x80000, nor the driver filter and that width exactly. Then its flags lose 0x10, its s16 sizes +0x14 / +0x16 are
// (side + 1) >> 1 (arithmetic), its key is set as TexCacheItem_SetKey does (inlined) and +0x18dc takes the area's change (s16 by
// s16); below limit the loop ends.
void __fastcall TexCache_ShrinkItems(u8 *cache, void *, u8 **items, s32 count, s32 tier, s32 limit);

// 0x58d1d0 (thiscall on the cache; side, made, format, levels; pops 0x10; name unconfirmed): new surfaces of the side while its budget
// (+0x1860 + 4 * index, index SurfacePool_SideIndex) covers their area: the format size (0x7022d8 indexed by [0x8bd554]) * side *
// side (32-bit), * 4 / 3 (signed) when levels > 1, kept at 0x8c4898. Each turn, unless one failed or the side has 0x100 entries
// (+0x1800 + 4 * index): slot 4 of the render device ([0x8bd52c]) makes a surface; one made has its slot 0 (side, side, format,
// levels) set it up: failing, its slot 9 (1) deletes it and the turns end; else a new entry (Mem_Alloc 0x20, TexCacheEntry_Ctor)
// holding it at +0 goes at +0x400 * index + 4 * count and the count rises. The budget then loses [0x8c4898] (or the area when
// that is 0) and the turns go on while it is at least [0x8c4898]. *made = the entries made; AnimTextures_Set(cache, cache +
// 0x1800); returns made > 0.
u32 __fastcall TexCache_AddSurfaces(u8 *cache, void *, s32 side, s32 *made, u32 format, s32 levels);

// 0x58d670 (thiscall; invented name, unconfirmed): the surfaces of the six sides (0x7022e8), two passes: a side with a budget (+0x1860 + 4 * i) adds
// surfaces; in the first pass one that adds none gives up the last entry of side index -1 or else 8 (SurfacePool_SideIndex of 0 and 1, sic) and tries
// again: failing, all freed (the debug log told) and 0. The budget left goes to the next side's; after each pass the last's goes to the first's.
// The debug log has the counts; +0x18ec = the first side with entries; 1.
u32 __fastcall TexCache_CreateSurfaces(u8 *cache);

// 0x58d400 (thiscall; invented name, unconfirmed): the budgets from the texture memory: the render device's slot 2 or, with none, what a run of 0x100
// surfaces reaches ([0x8c1620] or 0x2000000 to spend; freed again), kept at 0x8c1624. Too few surfaces for it (or none added): error kind 3 and 0.
// The mipmaps stay with flag 2 of +0x18cc, more than 0x40 surfaces and [0x8bd524] 1; else are off. Each size's budget = its share (+0x187c) of the memory; 1.
u32 __fastcall TexCache_ChooseBudget(u8 *cache);

// 0x58ea20 (thiscall on the cache; a, b, limit: two size indices; pops 0xc; name unconfirmed; 0x58f6ca asks it): each size's share
// (+0x1818 - +0x1830, 32-bit, over its count +0x1800; fild / fidiv at 24 bits, stored as floats); slot 3 of a's first entry's
// surface (+0x400 * a, its +0) answers a count. b when twice b's share is below a's (or unordered), b's +0x1818 + 10 is below a's
// and count * 3 is below limit * 4 (signed, 32-bit); else a.
s32 __fastcall TexCache_ChooseSize(u8 *cache, void *, s32 a, s32 b, s32 limit);

// 0x591010 (thiscall on a cache entry; an unused dword; pops 4; name unconfirmed; 0x58cd19 and 0x58cec2 ask it): +8 = slot 3 of the
// surface (+0) (its area), +0xc and +0x1c zeroed; then for each item of the list at +4 (next +4; flags +8, size s16 +0x14 by +0x16,
// area their product): bit 0x4000 marks the entry kept; an item with flags & 0x30 (and when it uses the driver filter,
// Texture_UsesDriverFilter, also width > [0x702564]) clears "whole" (set when the list has items), one using the driver filter sets
// "filtered"; the area adds to +0xc with 0x40000000, to +0x1c with 0x4000, to the sum always, times the dword at +0x10 past the
// item's virtual base (vbtable +0[4]) to a weighted sum; 0x10000000 items are counted. The surface's flags are set to kept | filtered
// << 1 | whole << 2 (TexCacheItem_SetFlags); +8 loses the sum. With age = [0x8bcc40] - +0x14, fresh = 0x10 - age below 0x10 (else 0):
// +0x10 = weighted + area * fresh + 2 * (items + 2 * (+0xc + 2 * (sum + 2 * counted))) (32-bit), which is returned.
s32 __fastcall TexCacheEntry_Score(u8 *entry, void *, u32);

// 0x590350 (thiscall on the cache; entry, place, texture; pops 0xc; name unconfirmed; 0x58f471 and 0x58f855 ask it): the texture (null:
// place +0x10's) put at (place +0, +4) of the entry's surface: its slot 7 (texture, x, y) first. A texture with flag 0x80000000 wider or
// taller (s16 +0x14 / +0x16) than its +0x10 / +0x12 is cut to them, x / y moved by half the excess (sar), and loses the flag. +0xc =
// [[0x87a248] + 4], flags lose 0x2000 and get 0x1000, +0x18 = the entry. Then its texel offsets +0x28 / +0x2c and extents +0x20 / +0x24
// (x87, the game's 24 bits) by the display driver kind (0x8bd524):
//   4: b, a = 0x702474's 0.0 or [0x8c1628] by flags 0x10000 / 0x20000 (with 0x30000), else [0x8c1630] / [0x8c1634] with 0x4000000 or
//      [0x8c1628]; +0x28 = b, +0x2c = a, +0x20 = width - 2b, +0x24 = height - 2a; Texture_UpdateWrapMask; +0x1c = (surface +0x10 * y +
//      x) * 4 (the place's own x and y) + the surface's slot 2.
//   else, with the cache's scale (+0x1900) and the surface's width / height (+8 / +0xc): flags 0x30000 (unless 0x400 with +0x3c 0):
//      +0x28 = scale / width * a, +0x2c = scale / height * b (a, b by 0x10000 / 0x20000 as above), +0x20 / +0x24 = 1.0 - twice
//      those; otherwise, from c, d, e, f = [0x8c1634], [0x8c1638], [0x8c163c], [0x8c1630] with 0x4000000 or [0x8c1628], 0.0, 0.0,
//      [0x8c1628]: +0x2c = (e + c + y) / height * scale, +0x28 = (d + f + x) / width * scale, +0x24 = (height' - 2c) / height *
//      scale and +0x20 = (width' - 2f) / width * scale (the texture's, cut); +0x1c = the surface's slot 2.
// +0xc = [0x8bcc40]. Returns 1.
u32 __fastcall TexCache_PlaceTexture(u8 *cache, void *, u8 *entry, const s32 *place, u8 *texture);

// 0x58cba0 / 0x58cc30 (thiscall on the cache, not used; entries, first, last; pops 0xc; names invented, unconfirmed): the entries' pointers first to last sorted by their signed +0xc / +0x10, the smallest first. The ends walk in
// unbounded: the middle's key the pivot, swapped when they have not crossed; the left part recursed into, the right part looped on.
void __fastcall TexCache_SortEntriesByKept(u8 *cache, void *, u8 **entries, s32 first, s32 last);
void __fastcall TexCache_SortEntriesByScore(u8 *cache, void *, u8 **entries, s32 first, s32 last);

// 0x58e1a0 (thiscall on the cache, not used; items, first, last; pops 0xc; name invented, unconfirmed): first below last: RecordSort_RunPointersUp at 0x8bcb98 by the items' key +0x38 (stride 0x60; its temp the address of the stack's first).
void __fastcall TexCache_SortItemsByKey(u8 *cache, void *, u8 **items, s32 first, s32 last);

// 0x58e790 (thiscall on the cache; list head pointer; pops 4; name invented, unconfirmed): the first 0x1000 items of the chain (next +4) sorted by key (TexCache_SortItemsByKey) and chained again, the greatest key the head.
// Items past those lose flags 0xf030 (+8) and drop out (sic).
void __fastcall TexCache_SortListByKey(u8 *cache, void *, u8 **list);

// 0x58e820 (thiscall on the cache, not used; list head pointer; pops 4; name invented, unconfirmed): likewise by the items' addresses (RecordSort_RunUp of the array's dwords, more than one), the lowest address the head.
void __fastcall TexCache_SortListByAddress(u8 *cache, void *, u8 **list);

// 0x58e110 (thiscall, the cache not used; name invented, unconfirmed): a byte 0x10 appended to "c:\\tcache.log" (no file: nothing).
void __fastcall TexCache_TraceUpdate(u8 *cache);

// 0x58ccc0 (thiscall; name invented, unconfirmed): each size's entries scored (cache flag 1, or the surface without its flag 1) and ordered. Cache flag 1: all by kept (+0xc). Else those
// without the surface's flag 1 partitioned first (the ends walk in, swapped, sic: a lone middle swaps with itself and the last index can pass it), sorted by kept. +0x1818 then the last entry
// from the top whose twice kept is at most its surface's slot 3, those up to it sorted by score (+0x10). Then scored again with the totals +0x18d0 (+8), +0x18d4 (kept), +0x18d8 (slot 3).
void __fastcall TexCache_RankEntries(u8 *cache);

// 0x58e390 (thiscall; name invented, unconfirmed): the pending list (+0x18c4) ordered: its first 0x1000 items; those past them reset to their wanted size and dropped. The bands +0x1904-0x1914 from the highest
// +0x30; sorted by +0x30; +0x18dc their wanted area; every tier (+0x18f8) shrunk with no limit; over 2/3 of the room (+0x18d8 less +0x18d4) or, flag 4, over +0x18fc: on through the tiers to 0xc, limit the room, until it fits;
// flag 4 and still over: from the last item, flag 0x8000 and its area off (flag 0x80000 kept). Sorted by key, the list rebuilt (the last the head), the items with flag 0x8000 reset instead.
void __fastcall TexCache_ArrangePending(u8 *cache);

// 0x58fcf0 (thiscall; returns 1; name invented, unconfirmed): only with [0x8bd524] 1. Each size's share (+0x1818 over its count) below 0.67: with 1.5 times it below the next larger's share, that many spare entries by
// wanted items (a short list; its forward move smears, sic) released, the fewest wanted last; below the next smaller's, entries by key released with their items halved. Anything released: TexCache_RankEntries and
// TexCache_ArrangePending again.
u32 __fastcall TexCache_Rebalance(u8 *cache);

// 0x58f190 (thiscall (list head pointer), pops 4; name invented, unconfirmed): the list's first item onto the next entry of the first size with room (-1: none or too wide; the item stays). Driver filter: the item alone, halved to the surface's
// width, or one size down when that size has at least the share free; PlaceTexture, the surface's flags 2 or 3 and 4 when it can not be halved. Else merged with the entry's items and packed, on to smaller sizes while all fit; TexCache_ChooseSize picks
// between the first and the last; the rows sorted by item, placed in address order. Slot 4 of the surface 0: returns 0; a failed placement also calls its slot 5. Returns Texture_Reload's answer (the entry).
s32 __fastcall TexCache_PlaceNext(u8 *cache, void *, u8 **list);

// 0x58f940 (thiscall; name invented, unconfirmed): +0x1830's six dwords zeroed; TexCache_PlaceNext for the pending list until empty or 0; -1: the head reset and dropped, going on. Items left reset, the list emptied. Returns the last answer.
s32 __fastcall TexCache_PlacePending(u8 *cache);

// 0x58cf40 (thiscall; name invented, unconfirmed): the frame's work. [0x8bd550] bit 7: the trace. No pending items: 1. [0x8bd508] & 0xc0000000: TexCache_Evict. Rank, arrange, rebalance, place; a 0 from the placing: EngineErrors_Push (2, 0xb0754011), 0.
u32 __fastcall TexCache_Update(u8 *cache);

// 0x58e010 (thiscall, pops 4; name unconfirmed): the trace of a touched item: "c:\\tcache.log" appended to or, the first time (the
// cache's flag 0x40000000 not set, then set), created with [0x8bd550]'s 0x40f0 bytes, [0x8bd508] and [0x8bcbc8] first; then a byte 1
// and the record {item, its flags +8, words +0x10 and +0x12, float +0x30 (through the x87)}; flushed and closed. No file: nothing.
void __fastcall TexCache_TraceItem(u8 *cache, void *, const u8 *item);

// 0x58c9f0 (thiscall, pops 8; name unconfirmed): the item (null or flag 0x2000000: nothing) is used this frame. With flags 0x400 and
// 0x1000, a +0x3c and a frame +0xc not [[0x87a248] + 4]: TexCache_UnlinkItem first. It gets flag 0x40000000 and its virtual base's
// frame slot counted (the byte at the slot [0x879d48] up to 0xff, with +0x10) and +0x14 |= 0x8000. Not placed (no flag 0x3000): +0x30 =
// weight, flag 0x2000, bits 4-5 of the flags = bits 6-7 (of those already set: ORed after they are cleared), +0x14 / +0x16 = the words
// +0x10 / +0x12 rounded up by the cache's mask +0x18e4, TexCacheItem_SetKey, and it goes on the cache's list +0x18c4. Placed: +0x30
// falls to weight; with [0x8bd508] & 0xc0000000 and flags 0x1000 and 0x4000 its entry's +0 gets TexCacheItem_SetFlag1. Flag 0x800000:
// the same for the item at +0x60. Last, [0x8bd550] bit 7: TexCache_TraceItem.
void __fastcall TexCache_TouchItem(u8 *cache, void *, u8 *item, float weight);
}

// The C library's file calls for TexCache_TraceItem (the bench's recorders; null: the game's).
struct TexCacheTraceEnv {
    void *(__cdecl *fopen)(const char *name, const char *mode);                 // 0x67b8a0
    u32(__cdecl *fwrite)(const void *data, u32 size, u32 count, void *file);    // 0x67b6d0
    s32(__cdecl *fflush)(void *file);                                           // 0x67d070
    s32(__cdecl *fclose)(void *file);                                           // 0x67b620
};
void TexCache_SetTraceEnv(const TexCacheTraceEnv *e);

struct AddrCall;
void TexCache_SetDestroyCalls(const AddrCall *table, u32 count);   // tests/fncmp/texcachedestroy.py
extern "C" {
// 0x58d090 (cdecl; name invented, unconfirmed): no cache at 0x8c4b44: 0. Else TexCache_ReleaseGraphics of it, the cache freed and the global cleared: 1.
u32 __cdecl TexCache_Destroy();
// 0x58dc10 (thiscall; name invented, unconfirmed): TexCache_ReleaseItems of [0x8c4b44] with 1, this one's six arrays freed (Object_FreeSixPointerArrays), the pool 0x8c47d8's surfaces released, this one's 0x20000000 at +0x18cc off. Returns 1.
u32 __fastcall TexCache_ReleaseGraphics(u8 *self);
}
