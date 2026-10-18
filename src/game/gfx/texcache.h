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
}
