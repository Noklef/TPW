#include "spriteframe.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../math/x87.h"
#include "../render/engineerrors.h"
#include "../render/quad.h"
#include "texcache.h"
#include "texture.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }   // __ftol (0x67a830)

// The item's quad (+4, read again for each vertex): vertex 0 copied to 1, 1 to 2, 2 to 3, a dword at a time.
void SpreadVertex(u8 *item) {
    for (u32 v = 0; v < 3; ++v) {
        u8 *const quad = at<u8 *>(item, 4);
        for (u32 k = 0; k < 0x10; k += 4)
            at<u32>(quad, 0x10 * v + 0x10 + k) = at<u32>(quad, 0x10 * v + k);
    }
}

// Not 0 and not a NaN (fcomp against 0: neither equal nor unordered).
bool NonZero(X87Reg x) { return x.v < 0.0 || x.v > 0.0; }

// Texture_DestroyArray (gfx/texture) when set.
void DestroyArray(u8 *array) {
    if (array)
        Texture_DestroyArray(array);
}

// The textures' cache items unlinked (the first texture's +0x40 cleared after each, sic), the array destroyed, +0 and +0x16 cleared.
void FreeTextures(u8 *image) {
    for (s32 i = 0; i < (s32)at<u16>(image, 0x16); ++i) {
        u8 *const texture = at<u8 *>(image, 0) + i * 0x60;
        if (at<u32>(texture, 8) & 0x1000)
            TexCache_UnlinkItem(*reinterpret_cast<u8 **>((uintptr_t)0x008c4b44), nullptr, texture);
        at<u32>(at<u8 *>(image, 0), 0x40) = 0;
    }
    DestroyArray(at<u8 *>(image, 0));
    at<u32>(image, 0) = 0;
    at<u16>(image, 0x16) = 0;
}

void __fastcall DestroyImage(void *image) { SpriteImage_Dtor(static_cast<u8 *>(image)); }

// A count-prefixed array of 0x18-byte images destroyed (0x588350 on each, the last first) and freed.
void ImageArray_Destroy(u8 *images) {
    CRT_VectorDtorIterator(images, 0x18, at<s32>(images, (u32)-4), DestroyImage);
    GameHeap_Free(images - 4);
}

}  // namespace

extern "C" {

u8 *__fastcall SpriteFrame_Init(u8 *frame) {
    at<u32>(frame, 0x10) = 0;
    at<u32>(frame, 0x24) = 0xffffffff;
    at<u32>(frame, 0x28) = 0;
    at<u32>(frame, 0x2c) = 0x3f800000;
    at<u32>(frame, 0x30) = 0;
    at<u32>(frame, 0x34) = 0;
    return frame;
}

u32 __fastcall SpriteImage_AllocPixels(u8 *image, void *, u32 width, u32 height) {
    const u32 pixel = reinterpret_cast<const u32 *>((uintptr_t)0x00701fe0)[*reinterpret_cast<const u32 *>((uintptr_t)0x008bd554)];
    at<u16>(image, 0xe) = (u16)height;
    at<u16>(image, 0xc) = (u16)width;
    at<u32>(image, 4) = TrackedGlobal_Alloc(pixel * (height & 0xffff) * (width & 0xffff));
    if (!at<u32>(image, 4)) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754000);
        return 0;
    }
    return 1;
}

u32 __fastcall SpriteImage_SetTexture(u8 *image, void *, u32 pixels, u32 palette, u32 source, u32 width, u32 height, u8 *texture,
                                      u32 flags) {
    if (texture == nullptr) {
        if (u8 *const array = static_cast<u8 *>(GameHeap_Alloc(100))) {
            at<s32>(array, 0) = 1;
            texture = array + 4;
            CRT_VectorVbaseCtorIterator(texture, 0x60, 1, reinterpret_cast<void(__fastcall *)(void *, void *, u32)>(&Texture_Ctor),
                                        reinterpret_cast<void *>((uintptr_t)0x00590a70));
        }
        at<u8 *>(image, 0) = texture;
        if (texture == nullptr) {
            EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754000);
            return 0;
        }
    } else {
        at<u8 *>(image, 0) = texture;
    }
    at<u32>(at<u8 *>(image, 0), 0x40) = source != 0 ? source : pixels;
    at<u32>(at<u8 *>(image, 0), 0x3c) = palette;
    at<u32>(at<u8 *>(image, 0), 8) = (palette != 0 ? 0x1000440u : 0x1000040u) | flags | 0x100;
    Texture_SetSize(at<u8 *>(image, 0), nullptr, width, height);
    at<u16>(at<u8 *>(image, 0), 0x34) = (u16)width;
    return 1;
}

u32 __fastcall SpriteImage_MakeTextures(u8 *image, void *, u32 x, u32 y) {
    const s32 tile = at<s32>(global<u8 *>(0x008c4b44), 0x18f0);
    const s32 across = (s32)(tile + at<u16>(image, 0x10) - 1) / tile;
    const s32 down = (s32)(tile + at<u16>(image, 0x12) - 1) / tile;
    const s32 count = down * across;
    u8 *textures = nullptr;
    if (u8 *const array = static_cast<u8 *>(GameHeap_Alloc((u32)count * 0x60 + 4))) {
        at<s32>(array, 0) = count;
        textures = array + 4;
        CRT_VectorVbaseCtorIterator(textures, 0x60, count, reinterpret_cast<void(__fastcall *)(void *, void *, u32)>(&Texture_Ctor),
                                    reinterpret_cast<void *>((uintptr_t)0x00590a70));
    }
    at<u8 *>(image, 0) = textures;
    if (textures == nullptr)
        return 0;
    at<u16>(image, 0x16) = (u16)(down * across);
    u32 offset = 0;
    for (s32 row = 0; row < down; ++row, y += (u32)tile) {
        const u32 height = row < down - 1 ? (u32)tile : (((u32)tile - 1) & ((u32)at<u16>(image, 0x12) - 1)) + 1;
        u32 column = 0;
        for (s32 col = 0; col < across; ++col, column += (u32)tile, offset += 0x60) {
            const u32 width = col < across - 1 ? (u32)tile : (((u32)tile - 1) & ((u32)at<u16>(image, 0x10) - 1)) + 1;
            at<u32>(at<u8 *>(image, 0) + offset, 8) = 0x5180200;
            Texture_SetSize(at<u8 *>(image, 0) + offset, nullptr, width, height);
            at<u16>(at<u8 *>(image, 0) + offset, 0x34) = at<u16>(image, 0xc);
            const u32 pixel = reinterpret_cast<const u32 *>((uintptr_t)0x00701fe0)[global<u32>(0x008bd554)];
            at<u32>(at<u8 *>(image, 0) + offset, 0x40) = (y * at<u16>(image, 0xc) + x + column) * pixel + at<u32>(image, 4);
        }
    }
    return 1;
}

u8 *__fastcall SpriteImage_Init(u8 *image) {
    for (u32 offset = 0xc; offset < 0x18; offset += 2)
        at<u16>(image, offset) = 0;
    at<u32>(image, 0) = 0;
    at<u32>(image, 4) = 0;
    return image;
}

void __fastcall SpriteFrames_MarkTextures(u8 *set) {
    for (s32 i = 0; i < at<s32>(set, 0); ++i) {
        u8 *texture = at<u8 *>(at<u8 *>(set, 4), i * 0x18);
        at<u32>(texture, 8) = (at<u32>(texture, 8) & ~0x100u) | 0x200;
    }
}

u32 __fastcall SpriteImage_FreeTextures(u8 *image) {
    if (at<u16>(image, 0x16))
        FreeTextures(image);
    return 1;
}

u32 __fastcall SpriteImage_FreeTexturesIfSet(u8 *image) {
    if (at<u8 *>(image, 0) && at<u16>(image, 0x16))
        FreeTextures(image);
    return 1;
}

u32 __fastcall SpriteImage_Release(u8 *image) {
    if (at<u32>(image, 4)) {
        if (at<u8 *>(image, 0) && at<u16>(image, 0x16))
            FreeTextures(image);
        TrackedGlobal_Free(at<u32>(image, 4));
        at<u32>(image, 4) = 0;
        return 1;
    }
    if (at<u8 *>(image, 0)) {
        Texture_Release(at<u8 *>(image, 0));
        DestroyArray(at<u8 *>(image, 0));
        at<u32>(image, 0) = 0;
    }
    return 1;
}

void __fastcall SpriteImage_Dtor(u8 *image) { SpriteImage_Release(image); }

u32 __fastcall SpriteFrames_Free(u8 *set) {
    if (u8 *const textures = at<u8 *>(set, 8)) {
        Texture_DestroyArray(textures);
        at<u32>(set, 8) = 0;
    }
    if (at<u8 *>(set, 4)) {
        for (s32 i = 0; i < at<s32>(set, 0); ++i)
            at<u32>(at<u8 *>(set, 4), i * 0x18) = 0;
        if (u8 *const images = at<u8 *>(set, 4))
            ImageArray_Destroy(images);
        at<u32>(set, 4) = 0;
    }
    if (at<u32>(set, 0xc)) {
        TrackedGlobal_Free(at<u32>(set, 0xc));
        at<u32>(set, 0xc) = 0;
    }
    at<u32>(set, 0) = 0;
    return 1;
}

void __fastcall SpriteFrame_SetSize(u8 *frame, void *, s32 width, s32 height) {
    if ((s32)at<s16>(frame, 0x34) != width || (s32)at<s16>(frame, 0x36) != height) {
        u8 *const image = at<u8 *>(frame, 0x28);
        if (image && at<u8 *>(image, 0) && at<u16>(image, 0x16))
            FreeTextures(image);
    }
    at<u16>(frame, 0x34) = (u16)width;
    at<u16>(frame, 0x36) = (u16)height;
}

void __fastcall SpriteFrame_SetSize30(u8 *frame, void *, s32 width, s32 height) {
    if ((s32)at<s16>(frame, 0x30) != width || (s32)at<s16>(frame, 0x30) != height) {  // (+0x30 again, sic)
        u8 *const image = at<u8 *>(frame, 0x28);
        if (image && at<u16>(image, 0x16))
            FreeTextures(image);
    }
    at<u16>(frame, 0x30) = (u16)width;
    at<u16>(frame, 0x32) = (u16)height;
}

u32 __fastcall SpriteFrame_PlaceQuad(u8 *frame, void *, float *quad, float width, float height) {
    const X87Reg w = NonZero(X87Reg(width)) ? X87Reg(width) : X87Reg(at<float>(frame, 0x2c)) * 20.0f * global<float>(0x008bcbcc);
    const float h = NonZero(X87Reg(height)) ? height : (float)(X87Reg(at<float>(frame, 0x2c)) * 20.0f);   // 0x70200c
    const u8 *const image = at<const u8 *>(frame, 0x28);
    const s32 offsetX = at<s16>(image, 0x14);
    const s32 offsetY = at<s16>(image, 0x16);
    float left = (float)(X87Reg(offsetX) / X87Reg((s32)at<u16>(image, 0x10)) * w);
    float right = (float)(X87Reg((s32)at<u16>(image, 0xc) + offsetX) / X87Reg((s32)at<u16>(image, 0x10)) * w);
    X87Reg bottom = X87Reg((s32)at<u16>(image, 0xe) + offsetY) / X87Reg((s32)at<u16>(image, 0x12)) * h;
    X87Reg top = X87Reg(offsetY) / X87Reg((s32)at<u16>(image, 0x12)) * h;
    const float corner[4] = {quad[0], quad[1], quad[2], quad[3]};
    u32 result = 0;
    const u32 flags = at<u32>(frame, 0x10);
    if (flags & 0x10) {
        left = (float)(X87Reg(left) * -1.0f);   // 0x702038
        right = (float)(X87Reg(right) * -1.0f);
        result = 1;
    }
    if (flags & 0x60) {
        top = top * -1.0f;
        bottom = bottom * -1.0f;
        result ^= 1;
    }
    const X87Reg x0 = X87Reg(left) + quad[0];
    const X87Reg x1 = X87Reg(right) + quad[4];
    const float x0f = (float)x0;
    quad[0] = (float)x0;
    quad[12] = x0f;
    quad[4] = (float)x1;
    quad[8] = (float)x1;
    if ((at<u32>(frame, 0x10) & 0x200) && (global<u32>(0x008bcbc8) & 0x1000000)) {
        const X87Reg inverse = X87Reg(1.0f) / quad[3];   // 0x702030
        const X87Reg dx = X87_SubR(inverse * x0f, X87Reg(global<float>(0x008bd4f0)));
        const X87Reg dy = X87_SubR(X87Reg(quad[1]) * inverse, X87Reg(global<float>(0x008bd4f4)));
        const X87Reg dxx = dx * dx;
        const X87Reg across = X87Reg(right) - left;
        const float dyf = (float)dy;
        const X87Reg dyy = dy * dyf;
        const X87Reg down = bottom - top;
        const float acrossf = (float)across;
        const X87Reg aa = across * acrossf;
        const float downf = (float)down;
        const X87Reg dd = down * downf;
        const float aaf = (float)aa;
        const float dxxf = (float)dxx;
        const X87Reg reach = (dd + aaf) * 0.0002f;   // 0x70203c
        const float distance = (float)(dyy + dxxf);
        if (reach.v > distance) {
            const u8 *const picked = global<const u8 *>(0x008bd4fc);
            if (!picked || !(quad[3] >= at<float>(picked, 0xc)))
                global<u8 *>(0x008bd4fc) = frame;
        }
    }
    if (at<u32>(frame, 0x10) & 0x80000) {
        const X87Reg z0 = top + quad[2];
        const X87Reg z2 = bottom + quad[10];
        result ^= 1;
        const float z2f = (float)z2;
        quad[6] = (float)z0;
        quad[2] = (float)z0;
        quad[14] = (float)z2;
        quad[10] = z2f;
        for (u32 i = 1; i < 16; i += 4)
            quad[i] = (float)(X87Reg(global<float>(0x008bd4ec)) + quad[i]);
        if (at<u8>(frame, 0x10) & 0x80)
            Quad_TurnXZ(quad, at<float>(frame, 0x30));
        return result;
    }
    const X87Reg y0 = top + quad[1];
    const X87Reg y2 = bottom + quad[9];
    const float y2f = (float)y2;
    quad[5] = (float)y0;
    quad[1] = (float)y0;
    quad[13] = (float)y2;
    quad[9] = y2f;
    if (at<u8>(frame, 0x10) & 0x80)
        Quad_TurnXY(quad, at<float>(frame, 0x30), nullptr);
    if (at<u32>(frame, 0x10) & 0x1000)
        Quad_TurnXY(quad, global<float>(0x0087af60), corner);
    return result;
}

u32 __fastcall SpriteFrame_FillItem(u8 *frame, void *, u8 *item) {
    if (!(at<u32>(frame, 0x10) & 0x10000000))
        return 1;
    const u8 *const texture = *at<u8 **>(frame, 0x28);
    if (texture) {
        at<u32>(item, 0x10) = (at<u32>(texture, 8) & 0x1000) ? at<u32>(texture, 0x1c) : 0;
        at<u32>(item, 0xc) = Texture_FormatBits(texture);
        at<u32>(item, 0x18) = X87_FloatThrough(at<u32>(texture, 0x28));
        at<u32>(item, 0x1c) = X87_FloatThrough(at<u32>(texture, 0x2c));
        at<float>(item, 0x20) = (float)(X87Reg(at<float>(texture, 0x20)) + X87Reg(at<float>(item, 0x18)));
        at<float>(item, 0x24) = (float)(X87Reg(at<float>(texture, 0x24)) + X87Reg(at<float>(item, 0x1c)));
        if (at<u32>(texture, 8) & 0x100000)
            at<u32>(item, 0) |= 4;
    } else {
        at<u32>(item, 0x10) = 0;
        at<u32>(item, 0xc) = 0x1256;
        at<u32>(item, 0x24) = 0;
        at<u32>(item, 0x20) = 0;
        at<u32>(item, 0x1c) = 0;
        at<u32>(item, 0x18) = 0;
    }
    at<u32>(item, 0) = 0;   // (sic: the flag 4 just set is lost)
    const u32 flags = at<u32>(frame, 0x10);
    if (flags & 0x400) {
        const X87Reg x = at<float>(frame, 0x14), z = at<float>(frame, 0x1c);
        const u8 *const terrain = global<u8 *>(0x00879d60);
        const u8 *const grid = at<u8 *>(terrain, 0);
        const s32 ix = Ftol((x - at<float>(grid, 8)) * at<float>(terrain, 0x10));
        const s32 iz = Ftol((z - at<float>(grid, 0xc)) * at<float>(terrain, 0x14));
        const s32 width = at<s32>(terrain, 8), depth = at<s32>(terrain, 0xc);
        s32 ix1 = ix + 1, iz1 = iz + 1;
        if (ix1 > width)
            ix1 = width;
        else if (ix1 < 0)
            ix1 = 0;
        if (iz1 > depth)
            iz1 = depth;
        else if (iz1 < 0)
            iz1 = 0;
        const X87Reg dz = z - (X87Reg((double)iz) * at<float>(grid, 0x14) + at<float>(grid, 0xc));
        const X87Reg dx = x - (X87Reg((double)ix) * at<float>(grid, 0x10) + at<float>(grid, 8));
        const u8 *const heights = at<u8 *>(grid, 0x28);
        const u32 row = (u32)iz * (u32)width;
        const X87Reg h00 = at<float>(heights, ((u32)ix + row) * 4);
        const X87Reg across = X87Reg(at<float>(heights, ((u32)ix1 + row) * 4)) - h00;
        const X87Reg down = X87Reg(at<float>(heights, ((u32)iz1 * (u32)width + (u32)ix) * 4)) - h00;
        const X87Reg zpart = dz * at<float>(terrain, 0x14) * down;
        const X87Reg xpart = dx * at<float>(terrain, 0x10) * across;
        at<float>(frame, 0x18) = (float)((zpart + xpart) + at<float>(heights, ((u32)ix + row) * 4) + global<float>(0x008bd4ec) +
                                         at<float>(frame, 0x18));
    }
    u32 placed;
    if (flags & 0x40000) {
        u8 *quad = at<u8 *>(item, 4);
        at<u32>(quad, 0) = at<u32>(frame, 0x14);
        at<u32>(quad, 4) = at<u32>(frame, 0x18);
        at<u32>(quad, 8) = at<u32>(frame, 0x1c);
        quad = at<u8 *>(item, 4);
        at<u32>(quad, 4) = X87_FloatThrough(at<u32>(quad, 4)) ^ 0x80000000u;
        at<u32>(at<u8 *>(item, 4), 0xc) = 0x3f800000;   // 1.0
        quad = at<u8 *>(item, 4);
        if (at<float>(quad, 8) > global<float>(0x00702040))
            at<float>(quad, 8) = (float)(X87Reg(at<float>(quad, 8)) - global<float>(0x00702040));
        at<u32>(item, 0x28) = 2;
        SpreadVertex(item);
        const float scale = (float)(X87Reg(at<float>(frame, 0x2c)) * global<float>(0x00702008));
        const float wide = (float)(X87Reg(global<float>(0x008bcbcc)) * scale);
        placed = SpriteFrame_PlaceQuad(frame, nullptr, at<float *>(item, 4), wide, scale);
    } else {
        at<u32>(item, 0x28) = 0;
        if (at<u32>(frame, 0x10) & 0x80000) {
            u8 *const quad = at<u8 *>(item, 4);
            at<u32>(quad, 0) = at<u32>(frame, 0x14);
            at<u32>(quad, 4) = at<u32>(frame, 0x18);
            at<u32>(quad, 8) = at<u32>(frame, 0x1c);
            SpreadVertex(item);
            placed = SpriteFrame_PlaceQuad(frame, nullptr, at<float *>(item, 4), 0.0f, 0.0f);
            using Slot = void(__cdecl *)(void *, u32, void *, void *, u32, u32, u32, u32);
            void *const transformer = global<void *>(0x0087b1d8);
            void *const out = at<void *>(item, 4);
            const Slot slot = reinterpret_cast<Slot>((*static_cast<void *const *const *>(transformer))[1]);
            slot(transformer, global<u32>(0x0087b088), out, out, 0x10, 0x10, 4, 0);
            at<u32>(item, 0xc) |= 0x800;
        } else {
            u8 *const quad = at<u8 *>(item, 4);
            for (u32 k = 0; k < 0x10; k += 4)
                at<u32>(quad, k) = at<u32>(frame, k);
            SpreadVertex(item);
            placed = SpriteFrame_PlaceQuad(frame, nullptr, at<float *>(item, 4), 0.0f, 0.0f);
            if ((at<u32>(frame, 0x10) & 0x200) && !(global<u32>(0x008bcbc8) & 0x2000)) {
                u8 *q = at<u8 *>(item, 4);
                const X87Reg last = at<float>(q, 0x38);
                at<float>(q, 0x38) = (float)X87_SubR(X87_DivR(last - global<float>(0x00702038), global<float>(0x00702044)), last);
                q = at<u8 *>(item, 4);
                at<u32>(q, 0x28) = at<u32>(q, 0x38);
                q = at<u8 *>(item, 4);
                at<u32>(q, 0x18) = at<u32>(q, 0x28);
                q = at<u8 *>(item, 4);
                at<u32>(q, 8) = at<u32>(q, 0x18);
            }
        }
    }
    at<u32>(item, 8) = at<u32>(frame, 0x24);
    if (placed)
        at<u32>(item, 0) |= 2;
    if (at<u32>(frame, 0x10) & 0x8000) {
        const u32 low = at<u32>(item, 0x1c);
        const u32 high = at<u32>(item, 0x24);
        at<u32>(item, 0x24) = X87_FloatThrough(low);
        at<u32>(item, 0x1c) = high;
    }
    return 1;
}
}
