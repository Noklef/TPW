#include "texset.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

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
}
