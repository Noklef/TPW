#include "tiny_d.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

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

u16 __fastcall Fn_4e0de0(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x30) = (u16)v0;
    return (u16)v0;
}

u32 __fastcall Fn_4f0150(u8 *o, void *, u32 v0) {
    at<u8>(o, 0x0) = at<u8>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0);
    at<u32>(o, 0x4) = 0x0;
    at<u32>(o, 0x8) = 0x0;
    at<u32>(o, 0xc) = 0x0;
    return (u32)(uintptr_t)o;
}

u8 __cdecl Fn_4f6010() {
    return 0x0;
}

u16 __fastcall Fn_509ae0(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x60) = (u16)v0;
    return (u16)v0;
}

u16 __fastcall Fn_50b080(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x0) = (u16)v0;
    return (u16)v0;
}

void __cdecl Fn_52c2a0() {
    global<u8>(0x00818c54) = 0x0;
}

u32 __cdecl Fn_52c6c0(u32 v0, u32 v1, u32 v2, u32 v3) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = global<u32>(0x008186e4);
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v1), 0x0) = global<u32>(0x008186e8);
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v2), 0x0) = global<u32>(0x00818c0c);
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v3), 0x0) = global<u32>(0x00818c08);
    return v3;
}

void __cdecl Fn_52fbd0() {
    global<u32>(0x00820a8c) = 0x0;
}

u32 __fastcall Fn_564730(u8 *o, void *, u32 v0, u32 v1) {
    at<u32>(o, 0xc) = v0;
    at<u32>(o, 0x10) = v1;
    return v0;
}

u32 __stdcall Fn_56b620(u32 v0) {
    return v0;
}

u32 __fastcall Fn_5ce530(u8 *o, void *, u32 v0) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = at<u32>(at<u8 *>(o, 0x4), 0x0);
    return v0;
}

u32 __fastcall Fn_5cec50(u8 *o, void *, u32 v0, u32 v1) {
    return (u32)(uintptr_t)o;
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

u8 __cdecl Fn_5d1400() {
    return 0x1;
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

u32 __cdecl Fn_6590c3(u32 v0) {
    global<u32>(0x00faa5d4) = v0;
    return v0;
}

u32 __cdecl Fn_6590ea(u32 v0) {
    global<u32>(0x00faa5fc) = v0;
    return v0;
}

u32 __cdecl Fn_6593ff() {
    return global<u32>(0x0077c488);
}

u16 __cdecl Fn_6595a8() {
    return global<u16>(0x00faa5bc);
}

u32 __fastcall Fn_659770(u8 *o) {
    at<u32>(o, 0x0) = 0x7058f0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_659ce0(u8 *o) {
    at<u16>(o, 0x10) = 0x1;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_65a9d0(u8 *o) {
    at<u32>(o, 0x0) = 0x705938;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_65aa80(u8 *o, void *, u32 v0) {
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_65b850(const u8 *o) {
    return at<u32>(o, 0x148);
}

u32 __fastcall Fn_65bc70(const u8 *o) {
    return at<u32>(o, 0x90);
}

u32 __fastcall Fn_65d547(u8 *o, void *, u32 v0) {
    return at<u32>(o, 0x5c);
}

u32 __fastcall Fn_65d55a(const u8 *o) {
    return at<u32>(o, 0x60);
}

u32 __fastcall Fn_65d56b(const u8 *o) {
    return at<u32>(o, 0x54);
}

u32 __fastcall Fn_65d57c(const u8 *o) {
    return at<u32>(o, 0x48);
}

u32 __fastcall Fn_65d7bc(const u8 *o) {
    return at<u32>(o, 0x8c);
}

u32 __fastcall Fn_6617d0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x0) = 0x705938;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6617f0(u8 *o) {
    at<u16>(o, 0x12) = 0x1;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_661830(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x90) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_661850(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x94) = v0;
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

u32 __fastcall Fn_6618e6(u8 *o) {
    at<u8>(o, 0x4) = 0x1;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6618f8(u8 *o) {
    at<u8>(o, 0x4) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_661c0b(u8 *o) {
    at<u32>(o, 0x0) = 0x0;
    at<u16>(o, 0x4) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __cdecl Fn_661f9b(u32 v0) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x14) = 0x1;
    return v0;
}

u32 __fastcall Fn_662086(const u8 *o) {
    return at<u32>(o, 0xc);
}

u32 __fastcall Fn_662097(u8 *o) {
    at<u32>(o, 0x10) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6620cc(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x8) = v0;
    return (u32)(uintptr_t)o;
}

void __cdecl Fn_662411() {
    global<u32>(0x00faa638) = 0x1;
}

u32 __fastcall Fn_667801(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x140) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_66781a(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x144) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_668d84(const u8 *o) {
    return at<u32>(o, 0x148);
}

u32 __fastcall Fn_6693d0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x140) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_6693f0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x13c) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_669700(u8 *o) {
    at<u32>(o, 0x0) = 0x0;
    at<u32>(o, 0x4) = 0x0;
    at<u32>(o, 0x8) = 0x0;
    at<u32>(o, 0xc) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_669c30(const u8 *o) {
    return at<u32>(o, 0x10);
}

u32 __fastcall Fn_669c41(u8 *o) {
    at<u32>(o, 0x4) = 0x0;
    at<u32>(o, 0x0) = 0x0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_669f80(u8 *o) {
    at<u16>(o, 0x6) = 0x1;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_669f94(u8 *o) {
    at<u16>(o, 0x6) = 0x0;
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

u32 __fastcall Fn_66a002(u8 *o) {
    at<u16>(o, 0x4) = 0x1;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_66a281(const u8 *o) {
    return at<u32>(o, 0x138);
}

u32 __fastcall Fn_66a6c7(const u8 *o) {
    return at<u32>(o, 0x140);
}

u32 __fastcall Fn_66b469(const u8 *o) {
    return at<u32>(o, 0x140);
}

u32 __fastcall Fn_66b53e(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x148) = v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_66bff7(u8 *o) {
    at<u32>(o, 0x0) = 0x705938;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_66c922(const u8 *o) {
    return at<u32>(o, 0x138);
}

u32 __fastcall Fn_66cc7c(const u8 *o) {
    return at<u32>(o, 0x158);
}

u32 __fastcall Fn_66d950(const u8 *o) {
    return at<u32>(o, 0x140);
}

u32 __fastcall Fn_66dad2(const u8 *o) {
    return at<u32>(o, 0x13c);
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

u8 __cdecl Fn_6a65e0() {
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

u32 __stdcall Fn_6c0200(u32 v0, u32 v1) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v1), 0x0) = 0x0;
    return 0x0;
}

u32 __cdecl Fn_6c3900(u32 v0) {
    return v0;
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

u32 __stdcall Fn_6d2780(u32 v0, u32 v1) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v1), 0x0) = 0x0;
    return v1;
}

u32 __fastcall Fn_6d3b90(u8 *o) {
    at<u32>(o, 0x8) = 0x0;
    at<u16>(o, 0xc) = 0x0;
    at<u32>(o, 0x10) = 0x0;
    at<u32>(o, 0x14) = 0x0;
    at<u32>(o, 0x18) = 0x0;
    return (u32)(uintptr_t)o;
}

}
