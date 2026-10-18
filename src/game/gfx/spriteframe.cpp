#include "spriteframe.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../render/engineerrors.h"
#include "texcache.h"
#include "texture.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

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
}
