#include "texture.h"

#include <cstdint>
#include <initializer_list>
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "../render/engineerrors.h"
#include "texcache.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void __fastcall ReleaseElement(void *texture) { Texture_Release(static_cast<u8 *>(texture)); }

}  // namespace

extern "C" {

u32 __fastcall Texture_Alloc(u8 *texture, void *, u32 size, u32 flags) {
    const u32 pixel = reinterpret_cast<const u32 *>((uintptr_t)0x007024e0)[*reinterpret_cast<const u32 *>((uintptr_t)0x008bd554)];
    at<u32>(texture, 0x40) = TrackedGlobal_Alloc(pixel * size * size);
    if (!at<u32>(texture, 0x40)) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754000);
        return 0;
    }
    const s16 side = (s16)size;
    const s32 wide = side;
    at<s16>(texture, 0x14) = side;
    const u32 shifted = (flags << 6) | 0x1000000;   // (its AND AL,0xf8 clears nothing)
    at<s16>(texture, 0x10) = side;
    at<s16>(texture, 0x16) = side;
    at<s16>(texture, 0x12) = side;
    at<u32>(texture, 8) = shifted;
    const u32 level = wide > 0x80 ? 0 : wide > 0x40 ? 1 : wide > 0x20 ? 2 : wide > 0x10 ? 3 : wide <= 8 ? 5 : 4;
    const X87Reg lod = (X87Reg(1.5f) - at<float>(texture, 0x30)) * 2047.0f;  // 0x702530, 0x702534
    const u32 all = level | shifted;
    at<u32>(texture, 8) = all;
    const u32 key = (((u32)(long long)lod.v & 0xfff) << 16) + ((u32)wide << 8) + (u32)wide;
    at<u32>(texture, 0x38) = key;
    if (all & 0x4000)
        at<u32>(texture, 0x38) = key | 0x20000000;
    if (all & 0x80000)
        at<u32>(texture, 0x38) |= 0x40000000;
    at<s16>(texture, 0x34) = side;
    return 1;
}

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

u32 __fastcall Texture_Reload(u8 *texture) {
    global<u32>(0x008c4b60)++;
    at<u32>(texture, 0x14) = global<u32>(0x008bcc40);
    return vcall<u32>(at<u8 *>(texture, 0), 0x14);
}

u8 *__fastcall Texture_Lock(u8 *texture) {
    if (!(at<u32>(texture, 8) & 0x10000000))
        return at<u8 *>(texture, 0x40);
    u32 lock[4];
    lock[0] = global<u32>(0x00879a68);
    lock[1] = (u32)(s32)at<s16>(texture, 0x10);
    lock[2] = (u32)(s32)at<s16>(texture, 0x12);
    lock[3] = reinterpret_cast<const u32 *>((uintptr_t)0x007024e0)[global<u32>(0x008bd554)] * lock[1];
    vcall<void>(at<u8 *>(texture, 0x40), 8, lock);
    return global<u8 *>(0x00879a68);
}

u8 *__fastcall Texture_LockFlipped(u8 *texture) {
    u8 *const pixels = Texture_Lock(texture);
    if (!(at<u32>(texture, 8) & 0x40000))
        return pixels;
    const u32 pitch = reinterpret_cast<const u32 *>((uintptr_t)0x007024e0)[global<u32>(0x008bd554)] * (u32)(s32)at<s16>(texture, 0x10);
    if ((s16)(at<u16>(texture, 0x12) & 0xfffe) <= 0)
        return pixels;
    u8 *top = pixels;
    s32 row = 0;
    do {
        u32 *a = reinterpret_cast<u32 *>(top);
        u32 *b = reinterpret_cast<u32 *>(pixels + (u32)((s32)at<s16>(texture, 0x10) - row - 1) * pitch);  // sic: the width counts the rows
        u32 n = pitch >> 2;
        do {   // sic: a pitch under 4 bytes wraps the count
            const u32 t = *a;
            *a = *b;
            *b = t;
            a++;
            b++;
        } while (--n);
        top += pitch;
        row++;
    } while (row < (at<s16>(texture, 0x12) >> 1));
    return pixels;
}

u32 __fastcall Texture_Release(u8 *texture) {
    if (at<u32>(texture, 8) & 0x800000)
        Texture_Release(texture + 0x60);
    if (at<u32>(texture, 8) & 0x1000)
        TexCache_UnlinkItem(global<u8 *>(0x008c4b44), nullptr, texture);
    if (at<u32>(texture, 8) & 0x10000000) {
        if (u8 *const object = at<u8 *>(texture, 0x40))
            vcall<void>(object, 0, 1u);
    } else if (at<u32>(texture, 0x40)) {
        TrackedGlobal_Free(at<u32>(texture, 0x40));
        at<u32>(texture, 0x40) = 0;
    }
    if (at<u32>(texture, 0x3c)) {
        TrackedGlobal_Free(at<u32>(texture, 0x3c));
        at<u32>(texture, 0x3c) = 0;
    }
    const X87Reg lod = (X87Reg(1.5f) - at<float>(texture, 0x30)) * 2047.0f;  // 0x702530, 0x702534
    at<u16>(texture, 0x14) = 0;
    at<u16>(texture, 0x10) = 0;
    at<u16>(texture, 0x16) = 0;
    at<u16>(texture, 0x12) = 0;
    at<u32>(texture, 8) = 5;
    at<u32>(texture, 0x38) = ((u32)(long long)lod.v & 0xfff) << 16;
    return 1;
}
}

void Texture_DestroyArray(u8 *array) {
    CRT_VectorDtorIterator(array, 0x60, at<s32>(array, (u32)-4), ReleaseElement);
    GameHeap_Free(array - 4);
}
