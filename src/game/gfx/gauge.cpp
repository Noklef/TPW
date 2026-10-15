#include "gauge.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void setColour(u8 *gauge, u32 offset, const DrawColour &c) {
    std::memcpy(gauge + offset, &c.rgb, 4);
    gauge[offset + 4] = c.index;
}

}  // namespace

extern "C" {

u8 *__fastcall Gauge_Init(u8 *gauge) {
    for (u32 i = 0x30; i < 0x44; ++i) gauge[i] = 0;
    for (u32 i = 0; i < 0x30; i += 4) at<u32>(gauge, i) = 0;
    return gauge;
}

void __fastcall Gauge_SetMarksX(u8 *gauge, void *, u32 a, u32 b, u32 c) {
    at<u32>(gauge, 0x28) = c;
    at<u32>(gauge, 0x18) = a;
    at<u32>(gauge, 0x20) = b;
}

void __fastcall Gauge_SetMarksY(u8 *gauge, void *, u32 a, u32 b, u32 c) {
    at<u32>(gauge, 0x2c) = c;
    at<u32>(gauge, 0x1c) = a;
    at<u32>(gauge, 0x24) = b;
}

void __fastcall Gauge_SetRect(u8 *gauge, void *, s32 left, s32 top, s32 right, s32 bottom) {
    at<s32>(gauge, 0x10) = right;
    at<s32>(gauge, 0x14) = bottom;
    at<s32>(gauge, 0xc) = top;
    at<s32>(gauge, 8) = left;
    at<u32>(gauge, 4) = (u32)bottom - (u32)top;
    at<u32>(gauge, 0) = (u32)right - (u32)left;
}

void __fastcall Gauge_SetColour0(u8 *gauge, void *, DrawColour c) { setColour(gauge, 0x30, c); }
void __fastcall Gauge_SetColour1(u8 *gauge, void *, DrawColour c) { setColour(gauge, 0x35, c); }
void __fastcall Gauge_SetColour2(u8 *gauge, void *, DrawColour c) { setColour(gauge, 0x3a, c); }
void __fastcall Gauge_SetColour3(u8 *gauge, void *, DrawColour c) { setColour(gauge, 0x3f, c); }
}
