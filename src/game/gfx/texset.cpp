#include "texset.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../render/engineerrors.h"
#include "texman.h"
#include "texture.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *manager() { return global<u8 *>(0x0087a268); }

s32 &deferred() { return global<s32>(0x0087a670); }

u8 **deferredSets() { return reinterpret_cast<u8 **>((uintptr_t)0x0087a270); }

}  // namespace

extern "C" {

u32 __fastcall TexSet_InitShared(u8 *set, void *, u32 array) {
    at<u32>(set, 0) = 0;
    at<u32>(set, 4) = 0;
    at<u32>(set, 8) = 0;
    at<u16>(set, 0xc) = 0;
    at<u16>(set, 0xe) = 0;
    const u32 list = TrackedGlobal_Alloc(4);
    at<u32>(set, 8) = list;
    if (!list) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754000);
        return 0;
    }
    at<u16>(set, 0xc) = 0x8000;
    at<u16>(set, 0xe) = 1;
    *reinterpret_cast<u32 *>((uintptr_t)list) = array;
    return 1;
}

u8 *__fastcall TexSet_Ctor(u8 *set) {
    at<u32>(set, 0) = 0;
    at<u32>(set, 4) = 0;
    at<u32>(set, 8) = 0;
    at<u16>(set, 0xc) = 0;
    at<u16>(set, 0xe) = 0;
    return set;
}

u32 __fastcall TexSet_Texture(const u8 *set, void *, s32 i) { return at<u32 *>(set, 8)[i]; }

u32 *__fastcall TexSet_TextureSlot(const u8 *set, void *, s32 i) { return at<u32 *>(set, 8) + i; }

u32 __fastcall TexSet_Release(u8 *set) {
    if (--at<s16>(set, 0xe) != 0)
        return 0;
    if (at<u16>(set, 0xc) & 0x8000) {
        Texture_Release(at<u8 **>(set, 8)[0]);
        if (u8 *const array = at<u8 **>(set, 8)[0])
            Texture_DestroyArray(array);
    } else {
        for (s32 i = 0; i < at<s16>(set, 0xc); ++i)
            TexManager_ReleaseTexture(*reinterpret_cast<u8 *const *>((uintptr_t)0x0087a268), nullptr, at<u8 **>(set, 8)[i]);
    }
    TrackedGlobal_Free(at<u32>(set, 8));
    at<u32>(set, 8) = 0;
    return 1;
}

u32 __fastcall TexSet_LoadTextures(u8 *set) {
    const u8 *const def = at<const u8 *>(set, 0);
    const u8 *const info = at<const u8 *>(set, 4);
    for (s32 i = 0; i < at<s16>(set, 0xc); ++i) {
        if (at<u8>(def, 1) & 0x10)
            TexManager_SetLoadFlags(manager(), nullptr, 2);
        if (at<u8>(def, 1) & 0x20)
            TexManager_SetLoadFlags(manager(), nullptr, 4);
        if (at<u8>(def, 1) & 0x40)
            TexManager_SetLoadFlags(manager(), nullptr, 8);
        at<u8 **>(set, 8)[i] = TexManager_Acquire(manager(), nullptr, at<const char *>(info, 4), at<const char *>(info, 0xc) + 0x14 * i);
        if (u8 *const textures = at<u8 **>(set, 8)[i]) {
            if (at<u8>(def, 0) & 0x10)
                at<u32>(textures, 8) |= 0x10000;
            if (at<u8>(def, 0) & 0x20)
                at<u32>(at<u8 **>(set, 8)[i], 8) |= 0x20000;
        }
    }
    return 1;
}

void __cdecl TexSet_LoadDeferred() {
    for (s32 i = 0; i < deferred(); ++i)
        TexSet_LoadTextures(deferredSets()[i]);
    deferred() = 0;
}

u32 __fastcall TexSet_Init(u8 *set, void *, const u8 *def, const u8 *info) {
    if (at<u32>(set, 8)) {
        ++at<s16>(set, 0xe);
        for (s32 i = 0; i < at<s16>(set, 0xc); ++i)
            if (at<u8>(def, 0) & 0x30)
                at<u32>(at<u8 **>(set, 8)[i], 8) |= 0x30000;
        return 1;
    }
    at<const u8 *>(set, 0) = def;
    at<const u8 *>(set, 4) = info;
    at<u32>(set, 8) = TrackedGlobal_Alloc((u32)at<u16>(info, 0xa) * 4);
    if (!at<u32>(set, 8))
        return 0;
    if (at<u8>(def, 0) & 0x80) {
        u8 *textures = nullptr;
        if (u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x64))) {
            at<s32>(block, 0) = 1;
            textures = block + 4;
            CRT_VectorVbaseCtorIterator(textures, 0x60, 1, reinterpret_cast<void(__fastcall *)(void *, void *, u32)>(&Texture_Ctor),
                                        reinterpret_cast<void *>((uintptr_t)0x00590a70));
        }
        at<u16>(set, 0xc) = 0x8000;
        if (textures) {
            if (Texture_Alloc(textures, nullptr, at<u16>(at<const u8 *>(info, 0xc), 6), 1)) {
                at<u8 **>(set, 8)[0] = textures;
                at<u32>(at<u8 **>(set, 8)[0], 8) = 0x1000240;
                at<u16>(set, 0xe) = 1;
                return 1;
            }
            Texture_DestroyArray(textures);
        }
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754000);
        return 0;
    }
    at<u16>(set, 0xc) = at<u16>(info, 0xa);
    if (global<u32>(0x008bd550) & 0x40000000) {
        const s32 n = deferred();
        if (n < 0x100) {
            deferredSets()[n] = set;
            deferred() = n + 1;
        }
    } else {
        TexSet_LoadTextures(set);
    }
    at<u16>(set, 0xe) = 1;
    return 1;
}

u32 __fastcall TexSet_InitFromPair(u8 *set, void *, const u32 *pair) {
    return TexSet_Init(set, nullptr, reinterpret_cast<const u8 *>((uintptr_t)pair[0]), reinterpret_cast<const u8 *>((uintptr_t)pair[1]));
}
}
