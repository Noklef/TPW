#include "texcache.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../gfx/texture.h"
#include "../core/crt.h"
#include "../render/engineerrors.h"
#include "../render/texformat.h"
#include "land.h"
#include "terrain_blend.h"
#include "../gfx/texcache.h"
#include "../gfx/wavelet.h"
#include "../core/staticobjects.h"

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
    *reinterpret_cast<u32 *>((u32)(uintptr_t)&gTexSettings_8bd550) &= ~4u;
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
    const s32 memory = (*reinterpret_cast<s32 *>((uintptr_t)&gTexSettings_8bd550 + 0x40d4));
    const u32 unit = reinterpret_cast<const u32 *>(&gTexcache_701548)[*reinterpret_cast<const u32 *>((uintptr_t)&gTexSettings_8bd550 + 0x4)] << 20;
    if (video < 0x3000000 || memory < (s32)(unit * 3)) size = 0x10;
    if ((video < 0x4000000 || memory < (s32)(unit * 5)) && size == 0x40) size = 0x20;
    if (*reinterpret_cast<const u32 *>((uintptr_t)0x008bd524) == 4) return 1;
    u8 *const errors = gEngineErrors_879ad0;
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

u32 __fastcall TerrainTexCache_BlendTile(u8 *cache, void *, u8 *texture, u32 x, u32 y, const TerrainOverlay *overlay) {
    if (!Texture_Alloc(texture, nullptr, at<u32>(cache, 4), 0)) return 0;
    const u32 format = *reinterpret_cast<const u32 *>((uintptr_t)&gTexSettings_8bd550 + 0x4);
    u8 **const scratch = reinterpret_cast<u8 **>(&gWavelet_Block_879a68);
    if (gTexcache_701548[format] == 2) {
        TerrainBlend_Tile(at<const Lnd *>(cache, 0), nullptr, *scratch, x, y, overlay);
        const u32 side = at<u32>(cache, 4);
        const u32 *const from = reinterpret_cast<const u32 *>(*scratch);
        TexFormat_ConvertSquare(reinterpret_cast<u16 *>(Texture_Lock(texture)), from, side);
        return 1;
    }
    u8 *pixel = Texture_Lock(texture);
    TerrainBlend_Tile(at<const Lnd *>(cache, 0), nullptr, pixel, x, y, overlay);
    for (s32 i = 0; i < at<s32>(cache, 4) * at<s32>(cache, 4); ++i, pixel += 4) {
        const u8 b0 = pixel[0], b1 = pixel[1];
        pixel[0] = pixel[3];
        pixel[3] = b0;
        pixel[1] = pixel[2];
        pixel[2] = b1;
    }
    return 1;
}
}

namespace {

AddrCalls g_calls;

u32 Self(const void *p) { return (u32)(uintptr_t)p; }

// The tile's pixels blended for the record (the record's first dword, when set, its texture locked first).
void RedrawTile(const u8 *cache, u8 *tile, u8 *record, s32 x, s32 y) {
    if (at<u8 *>(record, 0)) at<u32>(record, 0) = g_calls.Th(0x00590da0, Self(at<u8 *>(record, 0)));   // Texture_LockFlipped
    const u32 format = *reinterpret_cast<const u32 *>((uintptr_t)&gTexSettings_8bd550 + 0x4);
    const u32 scratch = gWavelet_Block_879a68;
    if (gTexcache_701548[format] == 2) {
        g_calls.Th(0x0055f780, at<u32>(cache, 0), scratch, (u32)x, (u32)y, record);   // TerrainBlend_Tile
        g_calls.Cd(0x00562f70, g_calls.Th(0x00590d40, Self(tile)), scratch, at<u32>(cache, 4));   // TexFormat_ConvertSquare, Texture_Lock
    } else {
        g_calls.Th(0x0055f780, at<u32>(cache, 0), g_calls.Th(0x00590d40, Self(tile)), (u32)x, (u32)y, record);
        u8 *pixel = reinterpret_cast<u8 *>((uintptr_t)g_calls.Th(0x00590d40, Self(tile)));
        for (u32 left = at<u32>(cache, 4) * at<u32>(cache, 4); left != 0; --left, pixel += 4) {
            const u8 b0 = pixel[0], b1 = pixel[1];
            pixel[0] = pixel[3];
            pixel[3] = b0;
            pixel[1] = pixel[2];
            pixel[2] = b1;
        }
    }
    g_calls.Th(0x0058dc50, gTexcache_8c4b44, tile);   // TexCache_UnlinkItem
}

}  // namespace

void TerrainTexCache_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __fastcall TerrainTexCache_ReadLnd(u8 *cache, void *, const char *name) {
    g_calls.Th(0x0055f720, at<u32>(cache, 0), at<u32>(cache, 4));   // Lnd_SetTileSize
    if (!(u8)g_calls.Th(0x00560ff0, at<u32>(cache, 0), name)) {   // Lnd_Read
        g_calls.Th(0x0056af20, (u32)(uintptr_t)gEngineErrors_879ad0, 2u, 0xb0754001u);   // EngineErrors_Push
        return 0;
    }
    *reinterpret_cast<u32 *>((uintptr_t)&gTexSettings_8bd550) |= 4;
    return 1;
}

u32 __fastcall TerrainTexCache_Update(u8 *cache, void *, u32 *keys, u8 *records, s32 count) {
    u8 *mark = at<u8 *>(cache, 0x1000c);
    for (; count > 0; --count, ++keys, records += 0x14) {
        const s32 x = (s32)(*keys & 0xff), y = (s32)*keys >> 8;
        u8 **const slot = reinterpret_cast<u8 **>(cache + 8 + 4 * ((y << 7) + x));
        u8 *node = *slot;
        u8 *tile;
        bool redraw;
        if (node) {
            tile = at<u8 *>(node, 0xc);
            *keys = Self(tile);
            if (node == mark) {
                mark = at<u8 *>(mark, 4);
            } else {   // to just before the mark
                at<u8 *>(at<u8 *>(node, 0), 4) = at<u8 *>(node, 4);
                at<u8 *>(at<u8 *>(node, 4), 0) = at<u8 *>(node, 0);
                at<u8 *>(at<u8 *>(mark, 0), 4) = node;
                at<u8 *>(node, 0) = at<u8 *>(mark, 0);
                at<u8 *>(node, 4) = mark;
                at<u8 *>(mark, 0) = node;
                if (mark == at<u8 *>(cache, 0x1000c)) at<u8 *>(cache, 0x1000c) = at<u8 *>(mark, 0);
            }
            redraw = (at<u32>(records, 0x10) & 0x100) != 0;
        } else {   // the oldest taken over
            node = at<u8 *>(at<u8 *>(cache, 0x1000c), 0);
            at<u8 *>(cache, 0x1000c) = node;
            tile = at<u8 *>(node, 0xc);
            *keys = Self(tile);
            if (at<u32 *>(node, 8)) *at<u32 *>(node, 8) = 0;
            *slot = node;
            at<u8 **>(node, 8) = slot;
            redraw = true;
        }
        if (redraw) RedrawTile(cache, tile, records, x, y);
        g_calls.Th(0x00590f90, Self(tile));   // Texture_SetKeep
        g_calls.Th(0x0058c9f0, gTexcache_8c4b44, tile, 0u);   // TexCache_TouchItem
    }
    return 1;
}
}
