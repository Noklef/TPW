#include "device.h"

#include <cstdint>

namespace {

// The virtual base offset: the word at +4 of the object's vtable.
s32 BaseOffset(const u8 *object) { return *reinterpret_cast<const s32 *>(*reinterpret_cast<const uintptr_t *>(object) + 4); }

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall Class701260_Ctor(u8 *object) {
    at<u32>(object, 0) = 0x00701260;
    at<u32>(object, 4) = 0;
    at<u32>(object, 0x14) = 0xffffffff;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x1c) = 0;
    at<u32>(object, 0x20) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0x24) = 0;
    if (at<u32>(global<u8 *>(0x00879a24), 0x428) == 0x10de && global<u32>(0x008bd524) == 1)
        global<u32>(0x00879ab4) = 1;
    return object;
}

u8 *__fastcall Class701378_Ctor(u8 *object, void *, u32 mostDerived) {
    if (mostDerived != 0) {
        at<u32>(object, 0) = 0x00701378;
        Class701260_Ctor(object + 0x2c);
    }
    at<u32>(object, BaseOffset(object)) = 0x00701330;
    const s32 offset = BaseOffset(object);
    at<s32>(object, offset - 4) = offset - 0x2c;
    const u8 *vtable = at<const u8 *>(object, 0);
    at<u32>(object, 8) = 0;
    at<u32>(object, 0x10) = 2;
    at<u32>(object, 0x14) = 1;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 4) = 0;
    at<u32>(object, 0x20) = 0xffffffff;
    at<u32>(object, at<s32>(vtable, 4) + 0xc) = 0x3000;
    return object;
}
}
