#include "texture.h"

#include <cstdint>
#include <initializer_list>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall Texture_Ctor(u8 *texture, void *, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(texture, 0) = 0x00702538;
        for (u32 offset = 0x48; offset < 0x60; offset += 4)
            at<u32>(texture, offset) = 0;
    }
    at<u32>(texture, 0xc) = 0xffffffff;
    at<u32>(texture, 8) = 0;
    for (u32 offset = 0x10; offset < 0x18; offset += 2)
        at<u16>(texture, offset) = 0;
    for (u32 offset : {0x1cu, 0x24u, 0x20u, 0x2cu, 0x28u, 0x40u, 0x3cu, 4u, 0x18u, 0x30u})
        at<u32>(texture, offset) = 0;
    at<u16>(texture, 0x34) = 0;
    return texture;
}

u32 __fastcall Texture_FormatBits(const u8 *texture) {
    const u32 flags = at<u32>(texture, 8);
    u32 bits = Texture_UsesDriverFilter(texture) ? 0x400000 : 0;
    if (flags & 0x4000000)
        bits |= 0x7000000;
    switch (flags & 0x300) {
    case 0x100:
        return bits | 0x2756;
    case 0x200:
        return bits | 0x556;
    case 0x300:
        return bits | 0x200552;
    default:
        return bits | 0x1256;
    }
}

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
