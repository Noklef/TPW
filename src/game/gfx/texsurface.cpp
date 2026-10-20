#include "texsurface.h"

#include <cstdint>
#include <initializer_list>

#include "texatlas.h"
#include "texcache.h"
#include "texture.h"
#include "../core/vcall.h"
#include "../core/gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &back(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p - offset); }

// The virtual base's offset, from the vtable pointer at the object's start.
u32 BaseOffset(const u8 *surface) { return at<u32>(at<const u8 *>(surface, 0), 4); }

// Class B's virtual base offset from its sub-object (the vbtable pointer at ECX - 0x10).
u32 SubBaseOffset(const u8 *sub) { return at<u32>(back<const u8 *>(sub, 0x10), 4); }

// a / divisor as CDQ; IDIV: it faults (sic) on 0 or a quotient past 31 bits, as the originals.
s32 Idiv(s32 a, s32 divisor) {
    s32 q;
    __asm {
        mov eax, a
        cdq
        idiv divisor
        mov q, eax
    }
    return q;
}

}  // namespace

extern "C" {

u8 *__fastcall TexSurfaceA_Ctor(u8 *surface, void *, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(surface, 0) = 0x00702218;
        TexCacheItem_Ctor(surface + 0x3c);
    }
    at<u32>(surface, BaseOffset(surface)) = 0x007021f0;
    const u32 offset = BaseOffset(surface);
    at<u32>(surface, offset - 4) = offset - 0x3c;
    for (u32 i = 0; i < 4; i++) {
        at<u32>(surface, 4 + 4 * i) = 0;
        at<u32>(surface, 0x14 + 4 * i) = 0;
    }
    at<u32>(surface, 0x24) = 0;
    at<u32>(surface, BaseOffset(surface) + 8) = 0;
    at<u32>(surface, BaseOffset(surface) + 0xc) = 0;
    for (u32 i = 0; i < 4; i++)
        at<u32>(surface, 0x28 + 4 * i) = 0;
    return surface;
}

u8 *__fastcall TexSurfaceB_Ctor(u8 *surface, void *, u32 mostDerived) {
    if (mostDerived) {
        at<u32>(surface, 0) = 0x00702278;
        TexCacheItem_Ctor(surface + 0x10);
    }
    at<u32>(surface, BaseOffset(surface)) = 0x00702250;
    const u32 offset = BaseOffset(surface);
    at<u32>(surface, offset - 4) = offset - 0x10;
    at<u32>(surface, BaseOffset(surface) + 0xc) = 0;
    at<u32>(surface, BaseOffset(surface) + 8) = 0;
    at<u32>(surface, 8) = 0;
    at<u32>(surface, 4) = 0;
    return surface;
}

u32 __fastcall TexSurfaceA_Field38(const u8 *sub) { return back<u32>(sub, 0x38); }

u32 __fastcall TexSurfaceA_Area(const u8 *sub) {
    const u32 off = at<u32>(back<const u8 *>(sub, 0x3c), 4);
    return back<u32>(sub + off, 0x30) * back<u32>(sub + off, 0x34);
}

u32 __fastcall TexSurfaceA_Field10(const u8 *sub) { return back<u32>(sub, 0x10); }

u32 __fastcall TexSurfaceB_FieldC(const u8 *sub) { return back<u32>(sub, 0xc); }

u32 __fastcall TexSurfaceB_Area(const u8 *sub) {
    const u32 off = at<u32>(back<const u8 *>(sub, 0x10), 4);
    return back<u32>(sub + off, 8) * back<u32>(sub + off, 4);
}

float __fastcall TexSurfaceB_Zero(const u8 *) { return *reinterpret_cast<const float *>((uintptr_t)0x00702248); }

u32 __fastcall TexSurfaceB_FreePixels(u8 *sub) {
    if (const u32 pixels = back<u32>(sub, 0xc)) {
        const u32 off = at<u32>(back<const u8 *>(sub, 0x10), 4);
        if (!(back<u32>(sub + off, 0xc) & 0x10000000))
            TrackedGlobal_Free(pixels);
        back<u32>(sub, 0xc) = 0;
    }
    return 1;
}

u8 *__fastcall TexSurfaceB_DeletingDtor(u8 *sub, void *, u32 flags) {
    u8 *const surface = sub - 0x10;
    at<u32>(surface, BaseOffset(surface)) = 0x00702250;
    const u32 offset = BaseOffset(surface);
    at<u32>(surface, offset - 4) = offset - 0x10;
    u8 *const base = surface + BaseOffset(surface);
    reinterpret_cast<u32(__fastcall *)(u8 *)>(at<void **>(base, 0)[1])(base);
    at<u32>(surface, 0x10) = 0x00702198;
    if (flags & 1)
        GameHeap_Free(surface);
    return surface;
}

u32 __fastcall TexSurfaceB_AllocPixels(u8 *sub, void *, u32 width, u32 height, u32, u32) {
    if (back<u32>(sub, 0xc))
        return 0;
    const u32 fresh = TexAtlas_Alloc(&back<u32>(sub, 0xc), static_cast<s32>(width));
    *reinterpret_cast<u32 *>((uintptr_t)0x008c4898) = height * width * 4;
    if (!back<u32>(sub, 0xc))
        return 0;
    if (!fresh)
        back<u32>(sub + SubBaseOffset(sub), 0xc) |= 0x10000000;
    back<u32>(sub + SubBaseOffset(sub), 8) = width;
    back<u32>(sub + SubBaseOffset(sub), 4) = height;
    at<u32>(sub + SubBaseOffset(sub), 0) = 0x100;
    back<u32>(sub, 8) = reinterpret_cast<const u32 *>((uintptr_t)0x00702220)[*reinterpret_cast<const u32 *>((uintptr_t)0x008bd554)];
    return 1;
}

u8 *__fastcall Renderer_NewSurfaceA(void *) {
    u8 *surface = static_cast<u8 *>(GameHeap_Alloc(0x50));
    surface = surface ? TexSurfaceA_Ctor(surface, nullptr, 1) : nullptr;
    if (!surface) return nullptr;
    return surface + BaseOffset(surface);
}

u8 *__fastcall SoftRenderer_NewSurfaceB(void *) {
    u8 *surface = static_cast<u8 *>(GameHeap_Alloc(0x24));
    surface = surface ? TexSurfaceB_Ctor(surface, nullptr, 1) : nullptr;
    if (!surface) return nullptr;
    return surface + BaseOffset(surface);
}

u32 __fastcall TexSurfaceA_ReleaseSurfaces(u8 *sub) {
    u32 count = *reinterpret_cast<const u32 *>(sub - 0x10);
    if (count == 0)
        count = 1;
    for (u32 i = count; i-- > 0;) {
        for (u32 array : {0x38u, 0x28u}) {
            u8 *&surface = *reinterpret_cast<u8 **>(sub - array + 4 * i);
            if (surface != nullptr) {
                comcall<u32>(surface, 8);
                surface = nullptr;
            }
        }
    }
    *reinterpret_cast<u32 *>(sub - 0x18) = 0;
    return 1;
}

u32 __fastcall TexSurfaceA_Blit(u8 *sub, void *, u8 *texture, s32 x, s32 y) {
    const auto pitch = []() { return *reinterpret_cast<const u32 *>((uintptr_t)0x008c48b8); };
    const auto size = [&]() { return back<u32>(sub, 0x14); };
    const auto zero = [](uintptr_t p, u32 n) {
        for (u32 k = 0; k < n; ++k)
            reinterpret_cast<u8 *>(p)[k] = 0;
    };
    const uintptr_t base = (u32)y * pitch() + *reinterpret_cast<const u32 *>((uintptr_t)0x008c48cc) + (u32)x * size();
    s32 height = at<s16>(texture, 0x16);
    s32 width = at<s16>(texture, 0x14);
    s32 top = 0, left = 0;
    const s32 srcWidth = at<s16>(texture, 0x10);
    if (width > srcWidth) {   // cut to the source's width, centred; the margins zeroed
        const s32 boxWidth = width;
        at<u32>(texture, 8) |= 0x80000000;
        left = (boxWidth - srcWidth) >> 1;
        const u32 bytes = size();
        width = srcWidth;
        const u32 leftBytes = (u32)left * bytes, rightBytes = (u32)(boxWidth - left - srcWidth) * bytes, right = (u32)(srcWidth + left) * bytes;
        if (leftBytes)
            for (u32 row = (u32)height; row != 0;) {
                --row;
                zero(row * pitch() + base, leftBytes);
            }
        if (rightBytes)
            for (u32 row = (u32)(s32)at<s16>(texture, 0x16); row != 0;) {
                --row;
                zero(row * pitch() + right + base, rightBytes);
            }
    }
    const s32 srcHeight = at<s16>(texture, 0x12);
    if (height > srcHeight) {   // the same for the height
        height = srcHeight;
        const u32 rowBytes = size() * (u32)(s32)at<s16>(texture, 0x14);
        at<u32>(texture, 8) |= 0x80000000;
        top = ((s32)at<s16>(texture, 0x16) - srcHeight) >> 1;
        if (top)
            for (u32 row = (u32)top; row != 0;) {
                --row;
                zero(row * pitch() + base, rowBytes);
            }
        u32 bottom = (u32)((s32)at<s16>(texture, 0x16) - srcHeight - top);
        if (bottom)
            for (u32 row = (u32)(s32)at<s16>(texture, 0x16); bottom != 0; --bottom) {
                --row;
                zero(row * pitch() + base, rowBytes);
            }
    }
    uintptr_t dst = (u32)left * size() + (u32)top * pitch() + base;
    if (at<u32>(texture, 8) & 0x10000000) {
        u32 record[4] = {(u32)dst, (u32)width, (u32)height, pitch()};
        if (vcall<u32>(at<u8 *>(texture, 0x40), 4, record, at<u32>(texture, 8) & 0x40000))
            return 1;
    }
    const s32 stepX = Idiv((s32)((u32)(s32)at<s16>(texture, 0x10) << 16), width);
    uintptr_t shade = at<u32>(texture, 0x3c);
    u32 frac = 0;
    s32 stepY = Idiv((s32)((u32)(s32)at<s16>(texture, 0x12) << 16), height);
    uintptr_t src = (uintptr_t)Texture_Lock(texture);
    const u32 bytes = size();
    const u32 srcPitch = (u32)(s32)at<s16>(texture, 0x34) * bytes;
    const u32 skip = pitch() - (u32)width * bytes;
    u32 mode = (at<u32>(texture, 8) & 0x400) && at<u32>(texture, 0x3c) ? 1 : 0;   // 1: shaded
    if (bytes == 4)
        mode |= 2;   // 4-byte pixels
    if (at<u32>(texture, 8) & 0x40000) {   // from the last row up
        const u32 last = (u32)(height - 1) * (u32)stepY;
        stepY = -stepY;
        const s32 rows = (s32)last >> 16;
        src += srcPitch * (u32)rows;
        shade += (u32)(s32)at<s16>(texture, 0x34) * (u32)rows;
        frac = last & 0xffff;
    }
    if (height == 0)
        return 1;
    const u32 *const tints = reinterpret_cast<const u32 *>((uintptr_t)0x00879e38);
    u32 rows = (u32)height;
    do {
        u32 fx = 0;
        u32 n = (u32)width;
        if (n) {
            do {
                const u32 i = (u32)((s32)fx >> 16);
                if (mode == 0) {
                    *reinterpret_cast<u16 *>(dst) = *reinterpret_cast<const u16 *>(src + i * 2);
                    dst += 2;
                } else if (mode == 1) {
                    const u32 t = tints[reinterpret_cast<const u8 *>(shade)[i]];
                    const u32 p = *reinterpret_cast<const u16 *>(src + i * 2);
                    const u32 r = ((t >> 16 & 0xff) * (p >> 8 & 0xf)) >> 8, g = ((t >> 8 & 0xff) * (p >> 4 & 0xf)) >> 8, b = ((t & 0xff) * (p & 0xf)) >> 8;
                    *reinterpret_cast<u16 *>(dst) = (u16)((((r << 4) + g) << 4) + b + (p & 0xf000));
                    dst += 2;
                } else if (mode == 2) {
                    *reinterpret_cast<u32 *>(dst) = *reinterpret_cast<const u32 *>(src + i * 4);
                    dst += 4;
                } else {
                    const u32 t = tints[reinterpret_cast<const u8 *>(shade)[i]];
                    const u32 p = *reinterpret_cast<const u32 *>(src + i * 4);
                    const u32 r = ((p >> 16 & 0xff) * (t >> 16 & 0xff)) >> 8, g = ((p >> 8 & 0xff) * (t >> 8 & 0xff)) >> 8, b = ((p & 0xff) * (t & 0xff)) >> 8;
                    *reinterpret_cast<u32 *>(dst) = (((r << 8) + g) << 8) + b + (p & 0xff000000);
                    dst += 4;
                }
                fx += (u32)stepX;
            } while (--n);
        }
        frac += (u32)stepY;
        const s32 step = (s32)frac >> 16;
        src += srcPitch * (u32)step;
        dst += skip;
        shade += (u32)(s32)at<s16>(texture, 0x34) * (u32)step;
        frac &= 0xffff;
    } while (--rows);
    return 1;
}

u32 __fastcall TexSurfaceB_Blit(u8 *sub, void *, u8 *texture, s32 x, s32 y) {
    const s32 height = at<s16>(texture, 0x16);
    uintptr_t dst = back<u32>(sub, 0xc) + (at<u32>(sub, SubBaseOffset(sub)) * (u32)y + (u32)x) * 4;
    const s32 width = at<s16>(texture, 0x14);
    const u32 flags = at<u32>(texture, 8);
    if (flags & 0x10000000) {
        u32 record[4] = {(u32)dst, (u32)width, (u32)height, at<u32>(sub, SubBaseOffset(sub)) << 2};
        if (vcall<u32>(at<u8 *>(texture, 0x40), 4, record, flags & 0x40000))
            return 1;
    }
    uintptr_t src = (uintptr_t)Texture_Lock(texture);
    const s32 stepX = Idiv((s32)((u32)(s32)at<s16>(texture, 0x10) << 16), width);
    const u32 skip = (at<u32>(sub, SubBaseOffset(sub)) - (u32)width) * 4;
    const u32 flip = at<u32>(texture, 8) & 0x40000;
    s32 stepY = Idiv((s32)((u32)(s32)at<s16>(texture, 0x12) << 16), height);
    u32 frac = 0;
    const u32 srcPitch = (u32)(s32)at<s16>(texture, 0x34) << 2;
    if (flip) {   // from the last row up
        const u32 last = (u32)(height - 1) * (u32)stepY;
        stepY = -stepY;
        src += (u32)((s32)last >> 16) * srcPitch;
        frac = last & 0xffff;
    }
    if (height == 0)
        return 1;
    u32 rows = (u32)height;
    do {
        u32 n = (u32)width;
        u32 fx = 0;
        if (n) {
            do {
                *reinterpret_cast<u32 *>(dst) = *reinterpret_cast<const u32 *>(src + (u32)((s32)fx >> 16) * 4);
                dst += 4;
                fx += (u32)stepX;
            } while (--n);
        }
        dst += skip;
        frac += (u32)stepY;
        src += (u32)((s32)frac >> 16) * srcPitch;
        frac &= 0xffff;
    } while (--rows);
    return 1;
}
}
