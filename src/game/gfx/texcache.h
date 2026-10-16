// The texture cache's items (the cache's log is c:\tcache.log; set up by 0x58da40, "Texture
// Cache Initialization"): the abstract base with vtable 0x702198 that its surfaces derive from
// virtually (0x58b3e0 builds one at +0x3c of a 0x702218 object). Reimplementation of the
// base's constructor and flag word methods 0x58b300-0x58b390 (thiscall).
//
// +4 is a flag word: bits 1, 2 and 4 are tested by the cache (0x58cd07, 0x58fe3d, 0x59007f);
// 0x10000000 survives SetFlags.
#pragma once

#include "../../../re/types.h"

extern "C" {

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
}
