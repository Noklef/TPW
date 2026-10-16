#include "small.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __cdecl Word_CopyTo(u16 *dest, const u16 *src) {
    if (dest)
        *dest = *src;
}

void __fastcall Vtbl703748_Set(u8 *object) { at<u32>(object, 0) = 0x00703748; }

void __fastcall Vtbl703770_Set(u8 *object) { at<u32>(object, 0) = 0x00703770; }

u8 *__fastcall Class703770_Ctor(u8 *object) {
    at<u32>(object, 0) = 0x00703770;
    at<u32>(object, 4) = 1;
    return object;
}

u32 __fastcall Obj_Get64Plus10(const u8 *object) { return at<u32>(object, 0x64) + 0x10; }

void __cdecl Stub_Ret2() {}

u32 __fastcall Obj_Get3c(const u8 *object) { return at<u32>(object, 0x3c); }

u32 *__cdecl Dwords_Copy(const u32 *first, const u32 *last, u32 *dest) {
    for (; first != last; first++, dest++)
        *dest = *first;
    return dest;
}

u8 *__fastcall Class703c80_Ctor(u8 *object) {
    at<u32>(object, 4) = 5;
    at<u32>(object, 8) = 5;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x10) = 1;
    at<u32>(object, 0x14) = 0xffffffff;
    at<u32>(object, 0x18) = 0xffffffff;
    at<u32>(object, 0x1c) = 0xffffffff;
    at<u32>(object, 0) = 0x00703c80;
    return object;
}

u32 __fastcall Obj_Take(u8 *object, void *, const u8 *source) {
    at<u32>(object, 0x14) = 0;
    at<u16>(object, 0xe) = at<u16>(source, 0xc);
    const u32 value = at<u32>(source, 0x10);
    at<u32>(object, 0x10) = value;
    at<u8>(object, 0xc) = 3;
    return value;
}

u32 __fastcall Obj_Set10(u8 *object, void *, u32 value) {
    at<u32>(object, 0x10) = value;
    return value;
}

s32 __cdecl DivCeil(s32 a, s32 b) { return (s32)((u32)a + (u32)b - 1) / b; }

u32 __cdecl Const_C0000() { return 0xc0000; }

u32 __cdecl Const_1000000() { return 1000000; }
}
