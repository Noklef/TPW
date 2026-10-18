#include "landscape.h"

#include <cstdint>

#include "../gfx/texture.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Landscape_SetPoint(u8 *landscape, void *, s32 i, u32 x, u32 y, u32 z) {
    u32 *point = reinterpret_cast<u32 *>(landscape + 0x1f70 + i * 12);
    point[0] = x;
    point[1] = y;
    point[2] = z;
}

void __fastcall Landscape_SetDetail(u8 *landscape, void *, u32 level) { at<u32>(landscape, 0x2ec8) = level; }

u32 __fastcall Landscape_List(const u8 *landscape) { return at<u32>(landscape, 0x1fd0); }

u8 *__fastcall Landscape_Point(u8 *landscape) { return landscape + 0x1fa0; }

u32 __fastcall Landscape_FreeTextures(u8 *landscape) {
    for (u32 offset = 0x1fd0; offset <= 0x1fd8; offset += 4) {
        if (!at<u8 *>(landscape, offset))
            continue;
        Texture_Release(at<u8 *>(landscape, offset));
        if (u8 *const array = at<u8 *>(landscape, offset))
            Texture_DestroyArray(array);
        at<u32>(landscape, offset) = 0;
    }
    return 1;
}

u32 __fastcall Landscape_RemoveTexture(u8 *landscape, void *, u8 *textures) {
    s32 i = at<s32>(landscape, 0x21dc);   // (from the one past the last, sic)
    while (i >= 0 && at<u8 *>(landscape, 0x1fdc + 0x20 * i) != textures)
        --i;
    if (i < 0)
        return 0;
    u8 *const entry = landscape + 0x1fdc + 0x20 * i;
    u8 *const array = at<u8 *>(entry, 0);
    at<u32>(entry, 0) = 0;
    if (array) {
        Texture_Release(array);
        Texture_DestroyArray(array);
    }
    const s32 last = --at<s32>(landscape, 0x21dc);
    if (last != 0) {
        u32 *const a = reinterpret_cast<u32 *>(entry);
        u32 *const b = reinterpret_cast<u32 *>(landscape + 0x1fdc + 0x20 * last);
        for (u32 k = 0; k < 8; ++k) {
            const u32 t = a[k];
            a[k] = b[k];
            b[k] = t;
        }
    }
    return 1;
}

u32 __fastcall Landscape_RemoveTextureList(u8 **list) {
    const s32 &count = *reinterpret_cast<const s32 *>(0x00785724);
    for (s32 i = 0; i < count; i++)
        Landscape_RemoveTexture(reinterpret_cast<u8 *>(0x008c1828), nullptr, list[i]);
    return static_cast<u32>(count);
}
}
