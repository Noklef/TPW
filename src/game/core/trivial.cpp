#include "trivial.h"

#include <cstdint>

#include "../files/file.h"
#include "gameheap.h"

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

u8 *__fastcall Pair_Assign(u8 *pair, void *, const u32 *dword, const u8 *byte) {
    const u32 value = *dword;
    at<u32>(pair, 0) = value;
    at<u8>(pair, 4) = *byte;
    return pair;
}

u32 __fastcall Obj_CopyCToMark(u8 *object) {
    const u32 value = at<u32>(object, 0xc);
    at<u32>(object, 0x38) = value;
    return (value & ~0xffu) | 1;
}

u32 __cdecl Const_C0000() { return 0xc0000; }

void __cdecl Stub_Ret2() {}

u32 __cdecl Const_1000000() { return 1000000; }

u32 __fastcall ReturnOne() { return 1; }

s32 __cdecl Const_MinusOne_564a50() { return -1; }

s32 __cdecl Const_MinusOne_5ceaa0() { return -1; }

s32 __stdcall Const_MinusOne_5cea90(u32) { return -1; }

s32 __stdcall Const_MinusOne_6bab20(u32, u32, u32, u32) { return -1; }

void __cdecl Debug_Nop_46cfb0() {}

u32 __cdecl Const1_424ec0() {
    return 0x1;
}

u32 __cdecl Const7ffffffd_4f6080() {
    return 0x7ffffffd;
}

u32 __cdecl Conste_4ff4f0() {
    return 0xe;
}

u32 __cdecl Constf_5061a0() {
    return 0xf;
}

u32 __stdcall Const1_50c1f0(u32 v0) {
    return 0x1;
}

u32 __stdcall Const1_569d00(u32 v0) {
    return 0x1;
}

u32 __stdcall Const1_56a430(u32 v0, u32 v1, u32 v2) {
    return 0x1;
}

u32 __stdcall Const1_56a440(u32 v0, u32 v1, u32 v2, u32 v3) {
    return 0x1;
}

u32 __cdecl Zero_5cec60() {
    return 0;
}

u32 __cdecl Zero_5d60c0() {
    return 0;
}

u32 __cdecl Zero_66f4d0() {
    return 0;
}

void __cdecl Stub_699ca0() {}

void __stdcall Stub_Ret8_6b70d0(u32, u32) {}

u32 __cdecl Const3_6b71c0() {
    return 0x3;
}

u32 __stdcall Const1_6b9bd0(u32 v0, u32 v1) {
    return 0x1;
}

u32 __cdecl Const4_6bd790() {
    return 0x4;
}

u32 __stdcall Zero_6c54a0(u32 v0) {
    return 0;
}

u32 __stdcall Zero_6c54b0(u32 v0, u32 v1, u32 v2, u32 v3) {
    return 0;
}
}
