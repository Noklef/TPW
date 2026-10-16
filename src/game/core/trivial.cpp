#include "trivial.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall Obj_Get3e4(const u8 *object) { return at<u32>(object, 0x3e4); }

u32 __fastcall Obj_Get3a4(const u8 *object) { return at<u32>(object, 0x3a4); }

u8 *__fastcall Obj_Self(u8 *object) { return object; }

u8 *__fastcall Obj_Part14(u8 *object) { return object + 0x14; }

u8 *__fastcall Obj_Part3c(u8 *object) { return object + 0x3c; }

void __stdcall Stub_Ret4(u32) {}

void __stdcall Stub_Ret12(u32, u32, u32) {}

u32 __fastcall Stub_One(void *) { return 1; }

void __cdecl Stub_Ret() {}

u32 __fastcall Obj_CopyCToMark(u8 *object) {
    const u32 value = at<u32>(object, 0xc);
    at<u32>(object, 0x38) = value;
    return (value & ~0xffu) | 1;
}
}
