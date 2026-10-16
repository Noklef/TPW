#include "tiny_g.h"

#include <cstdint>

#include "tiny_a.h"
#include "tiny_c.h"
#include "tiny_d.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 * __fastcall Wrap_Fn_669fa8_492020(u8 *o) {
    Fn_669fa8(reinterpret_cast<u8 *>(o)); at<u32>(o, 0x0) = 0x6ff598; at<u32>(o, 0x18) = 0x0; return o;
}

u32 __fastcall Wrap_Fn_669fee_492060(u8 *o) {
    at<u32>(o, 0x0) = 0x6ff598; return Fn_669fee(reinterpret_cast<u8 *>(o));
}

u32 __fastcall Wrap_Set4_4217f0_4fa460(u8 *o) {
    return Set4_4217f0(reinterpret_cast<u8 *>((o) + 0xc), nullptr, 0x3);
}

void __fastcall Wrap_Vtbl70a078_Set_6b4a50_6b0470(u8 *o) {
    at<u32>(o, 0x0) = 0x70a158; Vtbl70a078_Set_6b4a50(reinterpret_cast<u8 *>(o));
}

u8 * __fastcall Wrap_Init_6b4a60_6b0710(u8 *o) {
    Init_6b4a60(reinterpret_cast<u8 *>(o)); at<u32>(o, 0x0) = 0x70a188; return o;
}

void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b0750(u8 *o) {
    at<u32>(o, 0x0) = 0x70a188; Vtbl70a258_Set_6b4a90(reinterpret_cast<u8 *>(o));
}

u8 * __fastcall Wrap_Init_6b4a60_6b1de0(u8 *o) {
    Init_6b4a60(reinterpret_cast<u8 *>(o)); at<u32>(o, 0x0) = 0x70a1a8; return o;
}

void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b1e20(u8 *o) {
    at<u32>(o, 0x0) = 0x70a1a8; Vtbl70a258_Set_6b4a90(reinterpret_cast<u8 *>(o));
}

u8 * __fastcall Wrap_Init_6b4a60_6b2a10(u8 *o) {
    Init_6b4a60(reinterpret_cast<u8 *>(o)); at<u32>(o, 0x0) = 0x70a1c8; return o;
}

void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b2a50(u8 *o) {
    at<u32>(o, 0x0) = 0x70a1c8; Vtbl70a258_Set_6b4a90(reinterpret_cast<u8 *>(o));
}

u8 * __fastcall Wrap_Init_6b4a60_6b3c00(u8 *o) {
    Init_6b4a60(reinterpret_cast<u8 *>(o)); at<u32>(o, 0x0) = 0x70a1e8; return o;
}

void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b3c40(u8 *o) {
    at<u32>(o, 0x0) = 0x70a1e8; Vtbl70a258_Set_6b4a90(reinterpret_cast<u8 *>(o));
}

}
