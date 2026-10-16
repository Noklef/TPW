#include "tiny_c.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 * __fastcall Init_402f30(u8 *o) {
    at<u32>(o, 0x8) = 0;
    at<u32>(o, 0x0) = 0;
    at<u32>(o, 0xc) = 0;
    return o;
}

u8 * __fastcall CheatSystem_Construct(u8 *o) {
    at<u8>(o, 0xa) = 0;
    at<u8>(o, 0xb) = 0;
    at<u8>(o, 0x9) = 0;
    return o;
}

u8 * __fastcall Init_421510(u8 *o) {
    at<u32>(o, 0x0) = 0;
    at<u32>(o, 0x8) = 0;
    at<u32>(o, 0x4) = 0;
    return o;
}

u8 * __fastcall Init_4cf030(u8 *o) {
    at<u32>(o, 0x4) = 0;
    at<u8>(o, 0x11) = 0x1;
    at<u32>(o, 0x0) = 0;
    at<u32>(o, 0x8) = 0x5;
    at<u8>(o, 0x10) = 0;
    at<u32>(o, 0xc) = 0;
    return o;
}

u8 * __fastcall Init_4d0f70(u8 *o) {
    at<u32>(o, 0x0) = 0;
    at<u32>(o, 0x20) = 0;
    at<u32>(o, 0x24) = 0;
    at<u32>(o, 0x28) = 0;
    at<u8>(o, 0x2c) = 0;
    return o;
}

u8 * __fastcall Init_4d3ed0(u8 *o) {
    at<u32>(o, 0x0) = 0;
    at<u32>(o, 0x4) = 0;
    at<u32>(o, 0x8) = 0;
    return o;
}

u32 __fastcall Is19c_Eq1_4e0370(const u8 *o) {
    return at<u32>(o, 0x19c) == 0x1 ? 1 : 0;
}

u32 __fastcall Is19c_Eq4_4e0380(const u8 *o) {
    return at<u32>(o, 0x19c) == 0x4 ? 1 : 0;
}

u32 __fastcall Is19c_Eq2_4e03e0(const u8 *o) {
    return at<u32>(o, 0x19c) == 0x2 ? 1 : 0;
}

u32 __fastcall Is68_Zero_4e0440(const u8 *o) {
    return at<u32>(o, 0x68) == 0 ? 1 : 0;
}

u8 * __fastcall Init_4ef830(u8 *o) {
    at<u32>(o, 0x24) = 0x0;
    at<u32>(o, 0x0) = 0x709964;
    return o;
}

u16 __stdcall WordConstffff_4f06a0(u32 v0) {
    return 0xffff;
}

u16 __cdecl WordConstffff_4f06b0() {
    return 0xffff;
}

u8 * __fastcall Init_4f1890(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x4) = v0;
    at<u32>(o, 0x0) = 0x709b4c;
    return o;
}

u8 * __fastcall Init_4f1bd0(u8 *o) {
    at<u32>(o, 0x24) = 0x0;
    at<u32>(o, 0x0) = 0x7099e4;
    return o;
}

u8 * __fastcall Init_513290(u8 *o) {
    at<u32>(o, 0x0) = 0;
    at<u32>(o, 0x4) = 0;
    at<u8>(o, 0xc) = 0;
    return o;
}

u8 * __fastcall Init_5ac510(u8 *o) {
    at<u32>(o, 0x4) = 0;
    at<u32>(o, 0x8) = 0;
    at<u32>(o, 0xc) = 0;
    at<u32>(o, 0x0) = 0x7027f8;
    return o;
}

u8 * __fastcall Init_5cfd80(u8 *o) {
    at<u32>(o, 0x24) = 0x0;
    at<u32>(o, 0x0) = 0x709954;
    return o;
}

u8 * __fastcall Init_5d36c0(u8 *o) {
    at<u32>(o, 0x4) = 0xffffffff;
    at<u32>(o, 0x0) = 0x702994;
    return o;
}

u8 * __fastcall Init_5ec210(u8 *o) {
    at<u32>(o, 0x0) = 0;
    at<u32>(o, 0x4) = 0;
    at<u32>(o, 0x8) = 0;
    at<u32>(o, 0xc) = 0;
    return o;
}

u8 * __fastcall Init_691ee0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x7098dc;
    return o;
}

u8 * __fastcall Init_692970(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x70996c;
    return o;
}

u8 * __fastcall Init_69fef0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x4) = v0;
    at<u32>(o, 0x0) = 0x709d04;
    return o;
}

u32 __fastcall CopyTo_20_6a8cd0(u8 *o, void *, u32 v0) {
    at<u32>((void *)(uintptr_t)v0, 0) = at<u32>(o, 0x20);
    return v0;
}

u32 __fastcall CopyTo_24_6a8ce0(u8 *o, void *, u32 v0) {
    at<u32>((void *)(uintptr_t)v0, 0) = at<u32>(o, 0x24);
    return v0;
}

u32 __stdcall StoreThrough_ffffffff_6b4a00(u32 v0, u32 v1, u32 v2) {
    at<u32>((void *)(uintptr_t)v0, 0) = 0xffffffff;
    return v0;
}

u8 * __fastcall Init_6b4a60(u8 *o) {
    at<u32>(o, 0x0) = 0x70a238;
    return o;
}

u8 * __fastcall Init_6b9c50(u8 *o) {
    at<u32>(o, 0x0) = 0;
    at<u32>(o, 0x4) = 0;
    at<u32>(o, 0x40) = 0xa;
    at<u32>(o, 0x48) = 0;
    at<u32>(o, 0x4c) = 0;
    at<u32>(o, 0x68) = 0;
    return o;
}

u32 __fastcall SetThenFlag50_4c_6b9c70(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x50) = v0;
    at<u32>(o, 0x4c) |= 0x4;
    return at<u32>(o, 0x4c);
}

u32 __fastcall SetThenFlag60_4c_6b9cb0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x60) = v0;
    at<u32>(o, 0x4c) |= 0x20;
    return at<u32>(o, 0x4c);
}

u32 __fastcall SetThenFlag40_4c_6b9cf0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x40) = v0;
    at<u32>(o, 0x4c) |= 0x80;
    return at<u32>(o, 0x4c);
}

u32 __fastcall SetThenFlag44_4c_6b9d10(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x44) = v0;
    at<u32>(o, 0x4c) |= 0x100;
    return at<u32>(o, 0x4c);
}

u32 __fastcall SetThenFlag68_4c_6b9d30(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x68) = v0;
    at<u32>(o, 0x4c) |= 0x200;
    return at<u32>(o, 0x4c);
}

u8 * __fastcall Init_6c47b0(u8 *o) {
    at<u32>(o, 0x0) = 0x0;
    at<u32>(o, 0x4) = 0x0;
    return o;
}

u8 * __fastcall Init_6c4fb0(u8 *o) {
    at<u32>(o, 0x4) = 0;
    at<u32>(o, 0x8) = 0;
    at<u32>(o, 0xc) = 0;
    at<u32>(o, 0x0) = 0x70aa7c;
    at<u32>(o, 0x14) = 0;
    at<u8>(o, 0x18) = 0x1;
    return o;
}

u8 * __fastcall Init_6c4fd0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = v0;
    at<u32>(o, 0x4) = 0;
    at<u32>(o, 0x8) = 0;
    return o;
}

u32 __fastcall SetThenFlag48_14_6c5090(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x48) = v0;
    at<u32>(o, 0x14) |= 0x1000;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag8_14_6c50b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x8) = v0;
    at<u32>(o, 0x14) |= 0x4;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag9c_14_6c50d0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x9c) = v0;
    at<u32>(o, 0x14) |= 0x80000;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag4c_14_6c50f0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x4c) = v0;
    at<u32>(o, 0x14) |= 0x800;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag44_14_6c5110(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x44) = v0;
    at<u32>(o, 0x14) |= 0x40;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag28_14_6c5130(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x28) = v0;
    at<u32>(o, 0x14) |= 0x2;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag2c_14_6c5150(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x2c) = v0;
    at<u32>(o, 0x14) |= 0x400;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag24_14_6c5170(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x24) = v0;
    at<u32>(o, 0x14) |= 0x20;
    return at<u32>(o, 0x14);
}

u32 __fastcall FlagThenSet14_5c_6c51d0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x14) |= 0x10000;
    at<u32>(o, 0x5c) = v0;
    return v0;
}

u32 __fastcall FlagThenSet14_58_6c5220(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x14) |= 0x200000;
    at<u32>(o, 0x58) = v0;
    return v0;
}

u8 * __fastcall Init_6c6d30(u8 *o) {
    at<u32>(o, 0x4) = 0;
    at<u32>(o, 0x8) = 0;
    at<u32>(o, 0xc) = 0;
    at<u32>(o, 0x0) = 0x70abf8;
    return o;
}

u8 * __fastcall Init_6c9260(u8 *o) {
    at<u8>(o, 0x243c) = 0x0;
    at<u32>(o, 0x0) = 0x7118e0;
    return o;
}

u32 __stdcall StoreThrough_ffffffff_6d2790(u32 v0, u32 v1, u32 v2, u32 v3, u32 v4) {
    at<u32>((void *)(uintptr_t)v0, 0) = 0xffffffff;
    return v0;
}

u32 __stdcall StoreThrough_ffffffff_6d27a0(u32 v0) {
    at<u32>((void *)(uintptr_t)v0, 0) = 0xffffffff;
    return v0;
}

u32 __stdcall StoreThrough_ffffffff_6d29e0(u32 v0, u32 v1, u32 v2, u32 v3) {
    at<u32>((void *)(uintptr_t)v0, 0) = 0xffffffff;
    return v0;
}

u8 * __fastcall Init_6d6c50(u8 *o) {
    at<u32>(o, 0x4) = 0;
    at<u32>(o, 0x8) = 0x2;
    at<u32>(o, 0xc) = 0x5622;
    at<u32>(o, 0x10) = 0;
    at<u32>(o, 0x0) = 0x7119b8;
    return o;
}

}
