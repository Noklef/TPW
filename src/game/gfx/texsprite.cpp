#include "texsprite.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/ptrlist.h"
#include "color.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *const kFrames = reinterpret_cast<u8 *>((uintptr_t)0x00879ce0);

const u32 *Record(u32 address) { return reinterpret_cast<const u32 *>((uintptr_t)address); }

// The source's float at offset as a position (__ftol's low byte) blended across the three records at 0xf86c44.
u32 Blend(const u8 *source, u32 offset) {
    u32 out[9];
    return *Colour_Blend3(out, (s32)((u32)(long long)at<float>(source, offset) & 0xff), Record(0x00f86c44), Record(0x00f86c68), Record(0x00f86c8c));
}

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

void __fastcall TexSprite_Update(u8 *object, void *, u32 mask, u32 a, u32 b) {
    const u32 mode = at<u32>(object, 0x1c), key = at<u32>(object, 0x20);
    if (!((mode == 1 && (mask & key)) || (mode == 2 && a == key) || (mode == 3 && b == key))) {
        Obj_SetShown(object, nullptr, 0);   // (inlined)
        return;
    }
    Obj_SetShown(object, nullptr, 1);
    u32 colour = *Record(0x00f86a04);   // (the first of nine dwords copied)
    const u8 *const source = at<const u8 *>(object, 0x18);
    switch (at<u32>(object, 0x14)) {
    case 0:
        if (source && source[2] == 1)
            colour = Blend(source, 0x19c);
        break;
    case 1:
        colour = *Record(0x00f86cb0);
        break;
    case 3:
        colour = *Record(0x00f86cd4);
        break;
    case 4:
        if (source && source[2] == 6)
            colour = Blend(source, 0x1f8);
        break;
    case 5:
        colour = *Record(0x00f86cf8);
        break;
    }
    at<u32>(reinterpret_cast<u8 *>(Fn_56b620(at<u32>(object, 0x28))), 0x24) = colour;
}

u32 __fastcall Obj_SetShown(u8 *object, void *, u32 value) {
    if (at<u32>(object, 0x2c) == value)
        return value;
    at<u32>(object, 0x2c) = value;
    u8 *target = reinterpret_cast<u8 *>(Fn_56b620(at<u32>(object, 0x28)));
    at<u32>(target, 0x10) = value ? at<u32>(target, 0x10) & 0xfffffffe : at<u32>(target, 0x10) | 1;
    return (u32)(uintptr_t)target;
}
}
