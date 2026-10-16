#include "tiny_e.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall Fn_409cd0(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x0) = (u16)v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_4d2a60(u8 *o, void *, u32 v0) {
    at<u16>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = at<u16>(o, 0x10);
    return v0;
}

u32 __fastcall Fn_4e0df0(u8 *o, void *, u32 v0) {
    at<u16>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = at<u16>(o, 0x30);
    return v0;
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

u16 __fastcall Fn_657ddd(const u8 *o) {
    return at<u16>(o, 0xa);
}

u8 __fastcall Fn_659710(const u8 *o) {
    return at<u8>(o, 0x4);
}

u16 __fastcall Fn_659730(const u8 *o) {
    return at<u16>(o, 0xe0);
}

u16 __fastcall Fn_659790(const u8 *o) {
    return at<u16>(o, 0x4);
}

u32 __fastcall Fn_659994(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x8) = (u16)v0;
    return (u32)(uintptr_t)o;
}

u16 __fastcall Fn_6599ac(u8 *o, void *, u32 v0, u32 v1, u32 v2) {
    at<u16>(o, 0x2c) = (u16)v0;
    at<u16>(o, 0x2e) = (u16)v1;
    at<u16>(o, 0x30) = (u16)v2;
    return (u16)v1;
}

u32 __fastcall Fn_6599da(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x32) = (u16)v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_659a40(u8 *o, void *, u32 v0) {
    at<u16>(o, 0xe) = (u16)v0;
    return (u32)(uintptr_t)o;
}

u32 __fastcall Fn_659dac(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3) {
    at<u16>(o, 0x4) = (u16)v0;
    at<u16>(o, 0x6) = (u16)v1;
    at<u16>(o, 0x8) = (u16)v2;
    at<u16>(o, 0xa) = (u16)v3;
    return (u32)(uintptr_t)o;
}

u16 __fastcall Fn_65f330(const u8 *o) {
    return at<u16>(o, 0xe2);
}

u8 __fastcall Fn_6618c7(u8 *o, void *, u32 v0, u32 v1) {
    at<u8>(o, 0x5) = (u8)v0;
    at<u8>(o, 0x6) = (u8)v1;
    return (u8)v1;
}

u8 __fastcall Fn_66190a(const u8 *o) {
    return at<u8>(o, 0x4);
}

u32 __fastcall Fn_668564(u8 *o, void *, u32 v0) {
    at<u8>(o, 0x161) = (u8)v0;
    return (u32)(uintptr_t)o;
}

u8 __fastcall Fn_668800(const u8 *o) {
    return at<u8>(o, 0x161);
}

u16 __fastcall Fn_66a061(u8 *o, void *, u32 v0, u32 v1) {
    at<u16>(o, 0x8) = (u16)v0;
    at<u16>(o, 0xa) = (u16)v1;
    return (u16)v1;
}

u16 __fastcall Fn_66b454(const u8 *o) {
    return at<u16>(o, 0x14c);
}

u32 __fastcall Fn_66b523(u8 *o, void *, u32 v0) {
    at<u16>(o, 0x14c) = (u16)v0;
    return (u32)(uintptr_t)o;
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

u8 __stdcall Fn_6d2770(u32 v0) {
    at<u32>(reinterpret_cast<u8 *>((uintptr_t)v0), 0x0) = 0x0;
    return 0x0;
}

}
