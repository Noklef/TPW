#include "texture.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall Texture_UpdateWrapMask(u8 *texture) {
    s32 width = at<s16>(texture, 0x14);
    s32 height = at<s16>(texture, 0x16);
    if (width > 0 && (width & (width - 1)) == 0 && height > 0 && (height & (height - 1)) == 0)
        at<u32>(texture, 0x44) = (u32)(height - 1) << 8 | (u32)(width - 1);
    else
        at<u32>(texture, 0x44) = 0xffffffff;
}

u32 __fastcall Texture_Level(const u8 *texture) {
    u32 flags = at<u32>(texture, 8);
    if ((flags & 0x30) == 0x30)
        return flags & 7;
    s32 width = at<s16>(texture, 0x14);
    if (width > 0x80)
        return 0;
    if (width > 0x40)
        return 1;
    if (width > 0x20)
        return 2;
    if (width > 0x10)
        return 3;
    return width <= 8 ? 5 : 4;
}

void __fastcall Texture_ClearCacheFlags(u8 *texture) { at<u32>(texture, 8) &= ~0x7000u; }

void __fastcall Texture_SetKeep(u8 *texture) { at<u32>(texture, 8) |= 0x4000; }

void __fastcall Texture_ClearKeep(u8 *texture) { at<u32>(texture, 8) &= ~0x4000u; }

u32 __fastcall Texture_UsesDriverFilter(const u8 *texture) {
    if ((at<u32>(texture, 8) & 0x30000) == 0)
        return 0;
    return global<u32>(0x008bd524) == 1 ? 1 : 0;
}
}
