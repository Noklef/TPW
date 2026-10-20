#include "backdrop.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../gfx/texture.h"
#include "../gfx/texcache.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Backdrop_Clear5c(u8 *backdrop) { at<u32>(backdrop, 0x5c) = 0; }

u32 __fastcall Backdrop_Create(u8 *backdrop, void *, u8 *pixels, s32 width, s32 height, s32 pitch) {
    at<s32>(backdrop, 4) = width;
    at<s32>(backdrop, 0xc) = pitch;
    at<s32>(backdrop, 8) = height;
    const s32 across = (width + 63) / 64, down = (height + 63) / 64;
    at<s32>(backdrop, 0x14) = down;
    const s32 count = down * across;
    at<s32>(backdrop, 0x10) = across;
    u8 *tiles = nullptr;
    if (u8 *const block = static_cast<u8 *>(GameHeap_Alloc((u32)count * 0x60 + 4))) {
        at<s32>(block, 0) = count;
        tiles = block + 4;
        CRT_VectorVbaseCtorIterator(tiles, 0x60, count, reinterpret_cast<void(__fastcall *)(void *, void *, u32)>(&Texture_Ctor),
                                    reinterpret_cast<void *>((uintptr_t)0x00590a70));
    }
    at<u8 *>(backdrop, 0x1c) = tiles;
    const s32 bytes = *reinterpret_cast<const s32 *>((uintptr_t)0x00701820 + 4 * *reinterpret_cast<const u32 *>((uintptr_t)0x008bd554));
    const s32 rowPixels = at<s32>(backdrop, 0xc) / bytes;
    u32 made = 0;
    at<u32>(backdrop, 0x48) = 0;
    u8 *row = pixels;
    for (s32 y = 0; y < at<s32>(backdrop, 0x14); ++y) {
        u8 *p = row;
        for (s32 x = 0; x < at<s32>(backdrop, 0x10); ++x) {
            u8 *const tile = at<u8 *>(backdrop, 0x1c) + made * 0x60;
            at<u32>(tile, 8) = 0x5180000;
            Texture_SetSize(tile, nullptr, 64, 64);
            at<u16>(tile, 0x34) = (u16)rowPixels;
            at<u8 *>(tile, 0x40) = p;
            p += (u32)bytes << 6;
            ++made;
        }
        row += (u32)(rowPixels * bytes) << 6;
    }
    at<u32>(backdrop, 0x58) = made;
    return 1;
}

void __fastcall Backdrop_SetRect(u8 *backdrop, void *, u32 x0, u32 y0, u32 x1, u32 y1) {
    at<u32>(backdrop, 0) |= 2;
    at<u32>(backdrop, 0x20) = x0;
    at<u32>(backdrop, 0x24) = y0;
    at<u32>(backdrop, 0x28) = x1;
    at<u32>(backdrop, 0x2c) = y1;
}

void __fastcall Backdrop_SetSourceRect(u8 *backdrop, void *, u32 x0, u32 y0, u32 x1, u32 y1) {
    at<u32>(backdrop, 0x30) = x0;
    at<u32>(backdrop, 0x34) = y0;
    at<u32>(backdrop, 0x38) = x1;
    at<u32>(backdrop, 0x3c) = y1;
}

void __fastcall Backdrop_UncacheRect(u8 *backdrop, void *, s32 x0, s32 y0, s32 x1, s32 y1) {
    const s32 left = x0 / 64, top = y0 / 64, right = x1 / 64, bottom = y1 / 64;
    for (s32 y = top; y <= bottom; ++y)
        for (s32 x = left; x <= right; ++x) {
            u8 *const tile = at<u8 *>(backdrop, 0x1c) + (y * at<s32>(backdrop, 0x10) + x + at<s32>(backdrop, 0x48)) * 0x60;
            TexCache_UnlinkItem(*reinterpret_cast<u8 *const *>((uintptr_t)0x008c4b44), nullptr, tile);
        }
}
}
