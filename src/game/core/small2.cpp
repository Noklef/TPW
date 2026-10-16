#include "small2.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall Obj_GetBefore14c(const u8 *object) { return at<u32>(object, (u32)-0x14c); }

u32 __cdecl Global8bb628_SetOne() {
    global<u32>(0x008bb628) = 0x3f800000;
    return 0x3f800000;
}

u32 __cdecl Global8bba78_SetOne() {
    global<u32>(0x008bba78) = 0x3f800000;
    return 0x3f800000;
}

u32 __fastcall Obj_Set14_18_10(u8 *object, void *, u32 a, u32 b, u32 c) {
    at<u32>(object, 0x10) = c;
    at<u32>(object, 0x14) = a;
    at<u32>(object, 0x18) = b;
    return b;
}

u32 __cdecl Globals8bcb80_SetOne() {
    global<u32>(0x008bcb8c) = 0x3f800000;
    global<u32>(0x008bcb84) = 1;
    global<u32>(0x008bcb80) = 1;
    global<u32>(0x008bcb88) = 1;
    return 1;
}

u32 __fastcall Obj_SetColour8b4(u8 *object, void *, u32 colour) {
    at<u32>(object, 0x8b4) = colour;
    at<float>(object, 0x8b8) = (float)(colour & 0xff000000);
    at<float>(object, 0x8bc) = (float)(colour & 0xff0000);
    at<float>(object, 0x8c0) = (float)(colour & 0xff00);
    at<float>(object, 0x8c4) = (float)(colour & 0xff);
    return colour & 0xff;
}

}  // extern "C"
