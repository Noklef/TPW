#include "gauge.h"

#include <cstdint>
#include <cstring>

#include "display.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> const T &at(const void *p, u32 offset) { return *reinterpret_cast<const T *>((uintptr_t)p + offset); }

// The colour at gauge + offset (its rgb and index) as passed on.
DrawColour colourAt(const u8 *gauge, u32 offset) {
    DrawColour c = {};
    std::memcpy(&c.rgb, gauge + offset, 4);
    c.index = gauge[offset + 4];
    return c;
}

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

void __fastcall Gauge_DrawBack(const u8 *gauge) {
    const Rect frame = {at<s32>(gauge, 8), at<s32>(gauge, 0xc), at<s32>(gauge, 0x10), at<s32>(gauge, 0x14)};
    Display_BoxFill(&frame, colourAt(gauge, 0x3f));
}

void __fastcall Gauge_DrawAxes(const u8 *gauge) {
    const s32 bottom = at<s32>(gauge, 0x14), y0 = at<s32>(gauge, 0x1c);
    s32 y = bottom + 1;
    if (y0 < 0) {
        const s32 span = static_cast<s32>(at<u32>(gauge, 0x24) - static_cast<u32>(y0));
        if (span > 0) y = static_cast<s32>(at<u32>(gauge, 4) * static_cast<u32>(y0)) / span + bottom;
    }
    const s32 left = at<s32>(gauge, 8);
    Display_HLine(left - 1, y, at<s32>(gauge, 0x10) + 1, colourAt(gauge, 0x30));
    Display_VLine(left - 1, at<s32>(gauge, 0xc) - 1, bottom + 1, colourAt(gauge, 0x35));
}
}
