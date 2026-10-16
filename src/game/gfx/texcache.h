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
