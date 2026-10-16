#include "texcache.h"

#include <cstdint>
#include <initializer_list>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall TexCache_InitDefaults(u8 *cache) {
    static const struct { u32 offset, value; } kDefaults[] = {
        {0x18c0, 0x80}, {0x18bc, 0x80}, {0x18b8, 0x80}, {0x18b4, 0x80}, {0x18b0, 0x80},
        {0x187c, 0x3f000000}, {0x1880, 0x3e800000}, {0x1884, 0x3e2e147b}, {0x1888, 0x3d851eb8},
        {0x188c, 0x3c449ba6}, {0x1890, 0x3b449ba6}, {0x18ac, 0x100},
        {0x18f4, 0}, {0x1920, 0}, {0x18f8, 0}, {0x1900, 0x3f800000}, {0x18ec, 0}, {0x1878, 4},
        {0x18c4, 0}, {0x18c8, 0}, {0x18cc, 0}, {0x18d4, 0}, {0x18d0, 0}, {0x18e4, 7}, {0x18e8, 0},
    };
    for (const auto &d : kDefaults)
        at<u32>(cache, d.offset) = d.value;
    for (u32 i = 0; i < 6; i++) {
        at<u32>(cache, 0x1800 + 4 * i) = 0;
        at<u32>(cache, 0x1830 + 4 * i) = 0;
    }
    return cache;
}

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
