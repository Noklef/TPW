#include "texcache.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../gfx/texture.h"
#include "../core/crt.h"
#include "../render/engineerrors.h"
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

u32 __fastcall TerrainTexCache_Init(u8 *cache, void *, s32 count, u32 size) {
    if (size == 0) size = at<u32>(cache, 4);
    const s32 video = *reinterpret_cast<const s32 *>((uintptr_t)0x008bd520);
    const s32 memory = *reinterpret_cast<const s32 *>((uintptr_t)0x008c1624);
    const u32 unit = reinterpret_cast<const u32 *>((uintptr_t)0x00701548)[*reinterpret_cast<const u32 *>((uintptr_t)0x008bd554)] << 20;
    if (video < 0x3000000 || memory < (s32)(unit * 3)) size = 0x10;
    if ((video < 0x4000000 || memory < (s32)(unit * 5)) && size == 0x40) size = 0x20;
    if (*reinterpret_cast<const u32 *>((uintptr_t)0x008bd524) == 4) return 1;
    u8 *const errors = reinterpret_cast<u8 *>((uintptr_t)0x00879ad0);
    void *const block = GameHeap_Alloc(0x2ac);
    u8 *const land = block ? Lnd_Ctor(static_cast<u8 *>(block)) : nullptr;
    at<u8 *>(cache, 0) = land;
    if (land == nullptr) {
        EngineErrors_Push(errors, nullptr, 2, 0xb0754000);
        return 0;
    }
    std::memset(cache + 8, 0, 0x10000);
    const u32 bytes = (u32)count << 4;
    u8 *const tiles = static_cast<u8 *>(GameHeap_Alloc(bytes));
    at<u8 *>(cache, 0x10008) = tiles;
    if (tiles == nullptr) {
        EngineErrors_Push(errors, nullptr, 2, 0xb0754000);
        TerrainTexCache_Dtor(cache);
        return 0;
    }
    std::memset(tiles, 0, bytes);
    at<s32>(cache, 0x10010) = count;
    at<u32>(cache, 4) = size;
    for (s32 i = 0; i < count; ++i) {
        u8 *base = at<u8 *>(cache, 0x10008);
        at<u8 *>(base, i * 0x10) = i == 0 ? base + bytes - 0x10 : base + i * 0x10 - 0x10;
        base = at<u8 *>(cache, 0x10008);
        at<u8 *>(base, i * 0x10 + 4) = i == count - 1 ? base : base + (i + 1) * 0x10;
        u8 *textures = nullptr;
        if (u8 *const array = static_cast<u8 *>(GameHeap_Alloc(100))) {
            at<s32>(array, 0) = 1;
            textures = array + 4;
            CRT_VectorVbaseCtorIterator(textures, 0x60, 1, reinterpret_cast<void(__fastcall *)(void *, void *, u32)>(&Texture_Ctor),
                                        reinterpret_cast<void *>((uintptr_t)0x00590a70));
        }
        if (textures == nullptr || !Texture_Alloc(textures, nullptr, size, 0)) {
            EngineErrors_Push(errors, nullptr, 2, 0xb0754000);
            TerrainTexCache_Dtor(cache);
            return 0;
        }
        at<u8 *>(at<u8 *>(cache, 0x10008), i * 0x10 + 0xc) = textures;
    }
    at<u8 *>(cache, 0x1000c) = at<u8 *>(cache, 0x10008) + (count >> 1) * 0x10;
    return 1;
}
}
