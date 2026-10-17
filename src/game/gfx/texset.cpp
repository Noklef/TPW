#include "texset.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../render/engineerrors.h"
#include "texman.h"
#include "texture.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

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
}
