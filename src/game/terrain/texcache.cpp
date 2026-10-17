#include "texcache.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../gfx/texture.h"
#include "land.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall TerrainTexCache_Ctor(u8 *cache) {
    at<u32>(cache, 4) = 0x40;
    at<u32>(cache, 0x10008) = 0;
    at<u32>(cache, 0x10010) = 0;
    std::memset(cache + 8, 0, 0x10000);
    at<u32>(cache, 0) = 0;
    return cache;
}

u32 __fastcall TerrainTexCache_Dtor(u8 *cache) {
    *reinterpret_cast<u32 *>(0x008bd550) &= ~4u;
    if (at<u32>(cache, 0x10010)) {
        for (s32 i = 0; i < at<s32>(cache, 0x10010); ++i)
            if (u8 *const textures = at<u8 *>(at<u8 *>(cache, 0x10008), i * 0x10 + 0xc))
                Texture_DestroyArray(textures);
        GameHeap_Free(at<u8 *>(cache, 0x10008));
        at<u32>(cache, 0x10010) = 0;
    }
    if (u8 *const land = at<u8 *>(cache, 0)) {
        Lnd_Dtor(land);
        GameHeap_Free(land);
        at<u32>(cache, 0) = 0;
    }
    return 1;
}
}
