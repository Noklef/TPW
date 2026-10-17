#include "texsprite.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/ptrlist.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *const kFrames = reinterpret_cast<u8 *>((uintptr_t)0x00879ce0);

}  // namespace

extern "C" {

u8 *__fastcall TexSprite_Ctor(u8 *object, void *, u32 image, u32 a, u32 b, u32 c, u32 d) {
    at<u32>(object, 0) = 0x00703088;
    u8 *const frame = PtrArray_AddSpriteFrame(kFrames, nullptr, image);
    at<u32>(object, 0x14) = a;
    at<u32>(object, 0x1c) = b;
    at<u32>(object, 0x20) = c;
    at<u8 *>(object, 0x28) = frame;
    at<u32>(object, 0x2c) = 0;
    u8 *own = reinterpret_cast<u8 *>((uintptr_t)Fn_56b620((u32)(uintptr_t)frame));  // the array's identity accessor
    const u32 flags = at<u32>(own, 0x10);
    at<u32>(own, 0x10) = at<u32>(object, 0x2c) ? flags & ~1u : flags | 1;
    at<u32>(object, 0x24) = d;
    own = reinterpret_cast<u8 *>((uintptr_t)Fn_56b620(at<u32>(object, 0x28)));
    at<u32>(own, 0x10) |= 0x30000;
    PtrList_Append(*reinterpret_cast<PtrList **>((uintptr_t)0x00fa6e38), nullptr, object);
    return object;
}
}
