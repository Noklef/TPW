#include "texcache.h"

#include <cstdint>
#include <initializer_list>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall TexCacheEntry_Ctor(u8 *entry) {
    for (u32 offset : {0u, 4u, 0xcu, 8u})
        at<u32>(entry, offset) = 0;
    at<u32>(entry, 0x14) = 0xffffffff;
    return entry;
}

u8 *__fastcall TexCacheItem_Ctor(u8 *item) {
    at<u32>(item, 0) = 0x00702198;
    at<u32>(item, 4) = 0;
    return item;
}

void __fastcall TexCacheItem_SetFlags(u8 *item, void *, u32 flags) {
    at<u32>(item, 4) = (at<u32>(item, 4) & 0x10000000) | flags;
}

u32 __fastcall TexCacheItem_Flag1(const u8 *item) { return at<u32>(item, 4) & 1; }

u32 __fastcall TexCacheItem_Flag2(const u8 *item) { return at<u32>(item, 4) & 2; }

u32 __fastcall TexCacheItem_Flag4(const u8 *item) { return at<u32>(item, 4) & 4; }

void __fastcall TexCacheItem_SetFlag1(u8 *item) { at<u32>(item, 4) |= 1; }

void __fastcall TexCacheItem_ClearFlag1(u8 *item) { at<u32>(item, 4) &= ~1u; }
}
