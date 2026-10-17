#include "initializers.h"

#include <cstdint>
#include "crt.h"
#include "dequeiter.h"
#include "../gfx/surface.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The element constructor 0x507080 the array constructors pass (it clears +0, +0xc and the bytes +9 .. +0xb) and the element destructor, which is a call of the empty debug print.
void __fastcall Element507080Ctor(void *element) {
    u8 *const e = static_cast<u8 *>(element);
    at<u32>(e, 0xc) = 0;
    at<u32>(e, 0) = 0;
    e[9] = 0;
    e[0xa] = 0;
    e[0xb] = 0;
}

void __fastcall ElementNoDtor(void *) {}

}  // namespace

extern "C" {

u8 * __fastcall Init_402f30(u8 *o) {
    at<u32>(o, 0x8) = 0;
    at<u32>(o, 0x0) = 0;
    at<u32>(o, 0xc) = 0;
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

u8 * __fastcall Init_4ef830(u8 *o) {
    at<u32>(o, 0x24) = 0x0;
    at<u32>(o, 0x0) = 0x709964;
    return o;
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

u8 * __fastcall Init_6d6c50(u8 *o) {
    at<u32>(o, 0x4) = 0;
    at<u32>(o, 0x8) = 0x2;
    at<u32>(o, 0xc) = 0x5622;
    at<u32>(o, 0x10) = 0;
    at<u32>(o, 0x0) = 0x7119b8;
    return o;
}

u32 __fastcall Fn_47f910(u8 *o, void *, u32 v0) {
    at<u8>(o, 0x0) = at<u8>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0);
    at<u32>(o, 0x4) = 0x0;
    at<u32>(o, 0x8) = 0x0;
    at<u32>(o, 0xc) = 0x0;
    at<u32>(o, 0x10) = 0x0;
    at<u32>(o, 0x14) = 0x0;
    at<u32>(o, 0x18) = 0x0;
    at<u32>(o, 0x1c) = 0x0;
    at<u32>(o, 0x20) = 0x0;
    at<u32>(o, 0x24) = 0x0;
    at<u32>(o, 0x28) = 0x0;
    at<u32>(o, 0x2c) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_48c9e0(u8 *o) {
    at<u32>(o, 0x0) = 0x6ff1f0;
    global<u32>(0x007c2658) = 0x0;
    at<u32>(o, 0x10) = 0x0;
    at<u32>(o, 0x18) = 0x0;
    at<u32>(o, 0x14) = 0x0;
    at<u32>(o, 0x1c) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_4e0210(u8 *o) {
    at<u16>(o, 0x5e) = 0x0;
    at<u32>(o, 0x60) = 0x0;
    return 0x0;
}

u32 __fastcall Fn_4f0150(u8 *o, void *, u32 v0) {
    at<u8>(o, 0x0) = at<u8>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0);
    at<u32>(o, 0x4) = 0x0;
    at<u32>(o, 0x8) = 0x0;
    at<u32>(o, 0xc) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __cdecl Fn_52c6c0(u32 v0, u32 v1, u32 v2, u32 v3) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = global<u32>(0x008186e4);
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v1), 0x0) = global<u32>(0x008186e8);
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v2), 0x0) = global<u32>(0x00818c0c);
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v3), 0x0) = global<u32>(0x00818c08);
    return v3;
}

u32 __fastcall Fn_564730(u8 *o, void *, u32 v0, u32 v1) {
    at<u32>(o, 0xc) = v0;
    at<u32>(o, 0x10) = v1;
    return v0;
}

u32 __fastcall Fn_5d0e00(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5) {
    at<u32>(o, 0xc) = v0;
    at<u32>(o, 0x10) = v3;
    at<u32>(o, 0x1c) = v1;
    at<u32>(o, 0x20) = v4;
    at<u32>(o, 0x2c) = v2;
    at<u32>(o, 0x30) = v5;
    return v2;
}

u32 __stdcall Fn_5d1420(u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5, u32 v6) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v3), 0x0) = v1;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v6), 0x0) = v4;
    return 0x3;
}

u32 __fastcall Fn_5d29b0(u8 *o) {
    at<u32>(o, 0x0) = 0x700620;
    global<u32>(0x00f827ec) = 0x774178;
    global<u32>(0x00f827f0) = 0x7741e0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_5d5f70(u8 *o) {
    at<u32>(o, 0x18) = 0x0;
    at<u32>(o, 0x1c) = 0x0;
    at<u32>(o, 0x20) = 0x0;
    at<u32>(o, 0x28) = 0x0;
    return 0x0;
}

u32 __fastcall Fn_5e1bb0(u8 *o, void *, u32 v0, u32 v1, u32 v2) {
    at<u32>(o, 0x28) = v0;
    at<u32>(o, 0x2c) = v1;
    at<u32>(o, 0x30) = v2;
    return v2;
}

u32 __fastcall Fn_5ebe30(u8 *o, void *, u32 v0, u32 v1, u32 v2) {
    at<u32>(o, 0x0) = v0;
    at<u32>(o, 0x4) = v1;
    at<u32>(o, 0x8) = v2;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_5ebef0(u8 *o) {
    at<u8>(o, 0x0) = 0x0;
    at<u8>(o, 0x1) = 0x0;
    at<u8>(o, 0x2) = 0x0;
    at<u8>(o, 0x3) = 0x0;
    at<u32>(o, 0x4) = 0x0;
    at<u32>(o, 0x8) = 0x0;
    at<u32>(o, 0xc) = 0x0;
    at<u32>(o, 0x10) = 0x0;
    return 0x0;
}

u32 __cdecl Fn_5eda40(u32 v0, u32 v1) {
    global<u32>(0x00f86988) = v0;
    global<u32>(0x00f8698c) = v1;
    global<u32>(0x00f86990) = v0;
    global<u32>(0x00f86994) = v1;
    return v1;
}

u32 __cdecl Fn_64a450(u32 v0, u32 v1, u32 v2, u32 v3) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = global<u32>(0x0077a098);
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v1), 0x0) = global<u32>(0x0077a09c);
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v2), 0x0) = 0x77a0a0;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v3), 0x0) = 0x77b0a0;
    return 0x0;
}

u32 __cdecl Fn_64dd60(u32 v0) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = 0x64ddec;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x4) = 0x64de4f;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x8) = 0x64de13;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0xc) = 0x64dead;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x10) = 0x64dff8;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x68) = 0x0;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x6c) = 0x0;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x14) = 0x0;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x70) = 0x7054b8;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x74) = 0x77;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x78) = 0x0;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x7c) = 0x0;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x80) = 0x0;
    return v0;
}

u32 __fastcall Fn_657d30(u8 *o) {
    at<u32>(o, 0x0) = 0x7058d8;
    at<u32>(o, 0xc) = 0x0;
    at<u16>(o, 0x4) = 0x0;
    at<u16>(o, 0x6) = 0x2;
    at<u16>(o, 0x8) = 0x2;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_659770(u8 *o) {
    at<u32>(o, 0x0) = 0x7058f0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_65a9d0(u8 *o) {
    at<u32>(o, 0x0) = 0x705938;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6617d0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x705938;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_661870(u8 *o) {
    at<u32>(o, 0x0) = 0x7059b0;
    at<u8>(o, 0x5) = 0x0;
    at<u8>(o, 0x6) = 0x0;
    at<u8>(o, 0x4) = 0x1;
    at<u32>(o, 0x10) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6618a6(u8 *o) {
    at<u32>(o, 0x0) = 0x7059b0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_661c0b(u8 *o) {
    at<u32>(o, 0x0) = 0x0;
    at<u16>(o, 0x4) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_669700(u8 *o) {
    at<u32>(o, 0x0) = 0x0;
    at<u32>(o, 0x4) = 0x0;
    at<u32>(o, 0x8) = 0x0;
    at<u32>(o, 0xc) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_669c41(u8 *o) {
    at<u32>(o, 0x4) = 0x0;
    at<u32>(o, 0x0) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_669fa8(u8 *o) {
    at<u32>(o, 0x0) = 0x705a80;
    at<u16>(o, 0x4) = 0x1;
    at<u16>(o, 0x6) = 0x0;
    at<u32>(o, 0x14) = 0x0;
    at<u32>(o, 0x10) = 0x0;
    at<u16>(o, 0xc) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_669fee(u8 *o) {
    at<u32>(o, 0x0) = 0x705a80;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_66bff7(u8 *o) {
    at<u32>(o, 0x0) = 0x705938;
    return (u32)(uintptr_t)o;
}

u32 __cdecl Fn_67a610() {
    global<u32>(0x0077eca4) = 0x6805d0;
    global<u32>(0x0077eca0) = 0x680a30;
    global<u32>(0x0077eca8) = 0x680660;
    global<u32>(0x0077ecac) = 0x680570;
    global<u32>(0x0077ecb0) = 0x680640;
    global<u32>(0x0077ecb4) = 0x680a30;
    return 0x680a30;
}

u32 __cdecl Fn_682440(u32 v0) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x50) = 0x781490;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x14) = 0x1;
    return v0;
}

u32 __cdecl Fn_68bda0(u32 v0) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = 0x0;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x4) = 0x0;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x8) = 0x0;
    return 0x0;
}

u32 __fastcall Fn_6b52e0(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5) {
    at<u32>(o, 0x0) = v0;
    at<u32>(o, 0x4) = v1;
    at<u32>(o, 0x8) = v2;
    at<u32>(o, 0xc) = v3;
    at<u32>(o, 0x10) = v4;
    at<u32>(o, 0x14) = v5;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6b5310(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5) {
    at<u32>(o, 0x0) = v0;
    at<u32>(o, 0x4) = v1;
    at<u32>(o, 0x8) = v2;
    at<u32>(o, 0xc) = v3;
    at<u32>(o, 0x10) = v4;
    at<u32>(o, 0x14) = v5;
    return v4;
}

u32 __fastcall Fn_6b5430(u8 *o, void *, u32 v0, u32 v1) {
    at<u32>(o, 0x0) = v0;
    at<u8>(o, 0x4) = 0x0;
    at<u8>(o, 0x5) = (u8)v1;
    at<u32>(o, 0x8) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6bb220(u8 *o, void *, u32 v0, u32 v1, u32 v2) {
    at<u32>(o, 0x0) = v0;
    at<u32>(o, 0x4) = v1;
    at<u32>(o, 0x8) = v2;
    return v2;
}

u32 __fastcall Fn_6c54c0(u8 *o, void *, u32 v0, u32 v1) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v1), 0x0) = (u32)(uintptr_t)o;
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = 0x0;
    return v0;
}

u32 __fastcall Fn_6c58d0(u8 *o, void *, u32 v0, u32 v1) {
    at<u32>(o, 0x0) = v0;
    at<u32>(o, 0xc) = v1;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6c6a00(u8 *o, void *, u32 v0, u32 v1) {
    at<u32>(o, 0x4) = 0x0;
    at<u32>(o, 0x8) = 0x0;
    at<u32>(o, 0xc) = 0x0;
    at<u32>(o, 0x14) = v1;
    at<u32>(o, 0x0) = 0x70a678;
    at<u32>(o, 0x1c) = at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x20);
    at<u32>(o, 0x20) = at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x24);
    at<u32>(o, 0x0) = 0x70aba8;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6d1a50(u8 *o) {
    at<u32>(o, 0x2468) = at<u32>(o, 0x2474);
    at<u32>(o, 0x2430) = 0x0;
    at<u32>(o, 0x242c) = 0x0;
    at<u32>(o, 0x2464) = 0x0;
    at<u32>(o, 0x2410) = 0x0;
    return 0x0;
}

u32 __fastcall Fn_6d3b90(u8 *o) {
    at<u32>(o, 0x8) = 0x0;
    at<u16>(o, 0xc) = 0x0;
    at<u32>(o, 0x10) = 0x0;
    at<u32>(o, 0x14) = 0x0;
    at<u32>(o, 0x18) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_59b550(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x0) = at<u16>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0);
    at<u32>(o, 0xc) = at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0xc);
    at<u32>(o, 0x10) = at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x10);
    at<u8>(o, 0x14) = at<u8>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x14);
    at<u32>(o, 0x4) = at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x4);
    at<u32>(o, 0x8) = at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x8);
    at<u8>(o, 0x15) = at<u8>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x15);
    at<u8>(o, 0x16) = at<u8>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x16);
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_5f0970(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3) {
    at<u8>(o, 0x0) = (u8)v2;
    at<u8>(o, 0x1) = (u8)v1;
    at<u8>(o, 0x2) = (u8)v0;
    at<u8>(o, 0x3) = (u8)v3;
    at<u32>(o, 0x4) = 0x0;
    at<u32>(o, 0x8) = 0x0;
    at<u32>(o, 0xc) = 0x0;
    at<u32>(o, 0x10) = 0x0;
    return (u32)(uintptr_t)o;
}

u16 __fastcall Fn_6599ac(u8 *o, void *, u32 v0, u32 v1, u32 v2) {
    at<u16>(o, 0x2c) = (u16)v0;
    at<u16>(o, 0x2e) = (u16)v1;
    at<u16>(o, 0x30) = (u16)v2;
    return (u16)v1;
}

u32 __fastcall Fn_659dac(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3) {
    at<u16>(o, 0x4) = (u16)v0;
    at<u16>(o, 0x6) = (u16)v1;
    at<u16>(o, 0x8) = (u16)v2;
    at<u16>(o, 0xa) = (u16)v3;
    return (u32)(uintptr_t)o;
}

u8 __fastcall Fn_6618c7(u8 *o, void *, u32 v0, u32 v1) {
    at<u8>(o, 0x5) = (u8)v0;
    at<u8>(o, 0x6) = (u8)v1;
    return (u8)v1;
}

u16 __fastcall Fn_66a061(u8 *o, void *, u32 v0, u32 v1) {
    at<u16>(o, 0x8) = (u16)v0;
    at<u16>(o, 0xa) = (u16)v1;
    return (u16)v1;
}

u16 __fastcall Fn_66c096(u8 *o, void *, u32 v0, u32 v1, u32 v2) {
    at<u16>(o, 0x4) = (u16)v0;
    at<u16>(o, 0x6) = (u16)v1;
    at<u16>(o, 0x8) = (u16)v2;
    return (u16)v1;
}

u32 __fastcall Fn_66c1f3(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3) {
    at<u16>(o, 0x4) = (u16)v0;
    at<u16>(o, 0x6) = (u16)v1;
    at<u16>(o, 0x8) = (u16)v2;
    at<u16>(o, 0xa) = (u16)v3;
    return (u32)(uintptr_t)o;
}

u8 *__fastcall Class507800_Ctor(u8 *object) {
    CRT_VectorCtorIterator(object, 0x14, 0x20, Element507080Ctor, reinterpret_cast<void *>(ElementNoDtor));
    at<u32>(object, 0x294) = 0;
    at<u8>(object, 0x298) = 0;
    at<u32>(object, 0x299) = 0;
    at<u8>(object, 0x29d) = 0;
    return object;
}

void __fastcall Class507800_Dtor(u8 *object) {
    CRT_VectorDtorIterator(object, 0x14, 0x20, ElementNoDtor);
}

u8 *__fastcall Class7037e8_Ctor(u8 *object) {
    at<u32>(object, 0x0) = 0x703748;
    at<u32>(object, 0x4) = 0x703744;
    at<u32>(object, 0x8) = 0;
    at<u32>(object, 0xc) = 0;
    Class703c14_Ctor(object + 0x10);
    at<u32>(object, 0x54) = 1;
    at<u32>(object, 0x58) = 0;
    at<u32>(object, 0x5c) = 0;
    at<u32>(object, 0x60) = 0;
    at<u32>(object, 0x64) = 0;
    at<u32>(object, 0x68) = 0;
    at<u32>(object, 0x0) = 0x7037e8;
    return object;
}

u8 *__fastcall Class6fefc8_Ctor(u8 *object) {
    Fn_661870(object);
    for (u32 offset = 0x24; offset <= 0x28; offset++)
        at<u8>(object, offset) = 0;
    at<u32>(object, 0x34) = 0;
    at<u32>(object, 0x38) = 0;
    SurfaceInfo_Ctor(reinterpret_cast<SurfaceInfo *>(object + 0x3c));
    at<u32>(object, 0x0) = 0x6fefc8;
    at<u8>(object, 0x4) = 0;
    at<u32>(object, 0x8) = 0;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x64) = 0;
    return object;
}

u32 __fastcall Class6fefc8_Dtor(u8 *object) {
    at<u32>(object, 0x0) = 0x6fefc8;
    if (at<u8>(object, 0x7) != 0 && at<u32>(object, 0x64) != 0) {
        s32 result;
        Surface_Release(&result, reinterpret_cast<Surface *>(object + 0x14));
    }
    return Fn_6618a6(object);
}

u8 *__fastcall Class703710_Ctor(u8 *object) {
    Class7037e8_Ctor(object);
    at<u32>(object, 0x6c) = 0;
    at<u32>(object, 0x70) = 0;
    at<u32>(object, 0x74) = 0;
    at<u32>(object, 0x78) = 0;
    at<u32>(object, 0x80) = 0;
    at<u32>(object, 0x84) = 0x703730;
    at<u32>(object, 0x0) = 0x703710;
    return object;
}
}  // extern "C"
