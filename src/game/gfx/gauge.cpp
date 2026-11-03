#include "gauge.h"

#include <cstdint>
#include <cstring>

#include "../core/crt.h"
#include "../math/x87.h"
#include "../ui/window.h"
#include "display.h"
#include "guitext.h"
#include "palette.h"

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

s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }   // __ftol (0x67a830)

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

void __fastcall Gauge_DrawGrid(const u8 *gauge) {
    const s32 columns = at<s32>(gauge, 0x28), rows = at<s32>(gauge, 0x2c);
    if (!columns || !rows) return;
    const float dy = (float)(X87Reg(at<s32>(gauge, 4)) / X87Reg(rows));
    const X87Reg low = X87Reg(at<s32>(gauge, 0x14)) - dy;
    const float dx = (float)(X87Reg(at<s32>(gauge, 0)) / X87Reg(columns));
    float x = (float)(X87Reg(at<s32>(gauge, 8)) + dx);
    float y = (float)(low - -1.0f);   // (fsub [0x6ff06c], -1.0)
    s32 step = 0;   // (another column count leaves the original's as the stack lies)
    if (columns == 0xc)
        step = 0;
    else if (columns == 0x24)
        step = 2;
    else if (columns == 0x90)
        step = 0xb;
    s32 skipped = 0;
    for (s32 i = 0; i < at<s32>(gauge, 0x28); ++i) {
        if (++skipped > step) {
            skipped = 0;
            s32 left = Ftol(X87Reg(x));
            Display_VLine(left, at<s32>(gauge, 0xc), at<s32>(gauge, 0x14), colourAt(gauge, 0x3a));
            u16 label[4];
            Crt().swprintf(label, reinterpret_cast<const u16 *>(0x0074f9d8), i + 1);   // "%d"
            s32 top = at<s32>(gauge, 0x14) + 4;
            if (top < 2) top = 2;
            if (left < 2) left = 2;
            const s32 rect[4] = {left, top, left + 0x7d00, top + 0x7d00};
            GuiText_Draw(label, rect, gauge + 0x30, *reinterpret_cast<const s8 *>(0x0074f9d4), 0, 0, 0, nullptr, 1);
        }
        x = (float)(X87Reg(x) + dx);
    }
    for (s32 j = 0; j < at<s32>(gauge, 0x2c); ++j) {
        Display_HLine(at<s32>(gauge, 8), Ftol(X87Reg(y)), at<s32>(gauge, 0x10), colourAt(gauge, 0x3a));
        y = (float)(X87Reg(y) - dy);
    }
}

void __fastcall Gauge_DrawPlot(const u8 *gauge, void *, const s32 *samples, s32 count, s32 columns, DrawColour colour) {
    const float step = (float)(X87Reg(at<s32>(gauge, 0)) / X87Reg(columns));
    const s32 range = (s32)(at<u32>(gauge, 0x24) - at<u32>(gauge, 0x1c));
    float x = (float)at<s32>(gauge, 8);
    float next;
    std::memcpy(&next, &count, 4);   // (no sample: the count's slot as it is, read as a float)
    s32 y = columns;   // (no sample: the columns' slot, the label's top comes from it)
    for (s32 left = count; left > 0; --left, ++samples) {
        const s32 bottom = at<s32>(gauge, 0x14), top = at<s32>(gauge, 0xc);
        s32 y1 = bottom;
        y = bottom;
        if (range != 0) {
            const u32 y0 = at<u32>(gauge, 0x1c), height = at<u32>(gauge, 4);
            y1 = (s32)((u32)bottom - (u32)((s32)(((u32)samples[0] - y0) * height) / range));
            y = (s32)((u32)bottom - (u32)((s32)(((u32)samples[1] - y0) * height) / range));
        }
        const double low = at<s32>(gauge, 8), high = at<s32>(gauge, 0x10);
        next = (float)(X87Reg(step) + x);
        if (!((double)x >= low))
            x = (float)low;
        else if ((double)x > high)
            x = (float)high;
        if (!((double)next >= low))
            next = (float)low;
        else if ((double)next > high)
            next = (float)high;
        if (y1 < top)
            y1 = top;
        else if (y1 > bottom)
            y1 = bottom;
        if (y < top)
            y = top;
        else if (y > bottom)
            y = bottom;
        const s32 x2 = Ftol(X87Reg(next));
        Display_Line(Ftol(X87Reg(x)), y1, x2, y, colour);
        if (columns < 0x90) {   // (a small box round the point)
            const s32 a = (s32)((u32)x2 - 1), b = (s32)((u32)x2 + 1), t = (s32)((u32)y - 1), u = (s32)((u32)y + 1);
            Display_Line(a, t, b, t, colour);
            Display_Line(b, t, b, u, colour);
            Display_Line(b, u, a, u, colour);
            Display_Line(a, u, a, t, colour);
        }
        x = (float)(X87Reg(step) + x);
    }
    u16 label[12];
    Crt().swprintf(label, reinterpret_cast<const u16 *>(0x0074f9d8), *samples);   // "%d"
    s32 right = Ftol(X87Reg(next));
    s32 left = at<s32>(gauge, 8), top = (s32)((u32)y - 6);
    const s32 bottom = at<s32>(gauge, 0x14);
    const s8 font = *reinterpret_cast<const s8 *>(0x0074f9d4);
    const s32 measure[4] = {left, top, right, bottom};
    s32 box[4] = {};
    GuiText_Measure(label, measure, font, 2, 0, 0, box);
    if (top < 2) top = 2;
    if (left < 2) left = 2;
    const s32 over = (s32)((u32)box[2] + (u32)left - (u32)box[0] - (u32)right);
    if (over > 0) right = (s32)((u32)right + (u32)over);
    const s32 rect[4] = {left, top, right, bottom};
    GuiText_Draw(label, rect, reinterpret_cast<const u8 *>(&colour), font, 2, 0, 0, nullptr, 1);
}

void __cdecl Gauge_Setup(u8 *gauge, const u8 *window, u32 y0, u32 y1, u32 y2, u32 x0, u32 x1, u32 x2) {
    const s32 high = static_cast<s32>(static_cast<u32>(at<s16>(window, 0xa) - at<s16>(window, 6)) * static_cast<u32>(*reinterpret_cast<const s16 *>(0x00faa5c0)));
    const s32 wide = static_cast<s32>(static_cast<u32>(at<s16>(window, 8) - at<s16>(window, 4)) * static_cast<u32>(*reinterpret_cast<const s16 *>(0x00faa5c4)));
    Gauge_SetRect(gauge, nullptr, 4, 4, wide / 2048 - 20, high / 1536 - 20);
    const u8 *const palette = reinterpret_cast<const u8 *>(0x00fa71f8);
    DrawColour c = {};
    c.rgb = 0xff00c800;   // (blue, green, red, 0xff)
    c.index = Palette_Nearest(palette, 0, 0xc8, 0);
    Gauge_SetColour0(gauge, nullptr, c);
    c.index = Palette_Nearest(palette, 0, 0xc8, 0);
    Gauge_SetColour1(gauge, nullptr, c);
    c.rgb = 0xff000000;
    c.index = Palette_Nearest(palette, 0, 0, 0);
    Gauge_SetColour3(gauge, nullptr, c);
    c.rgb = 0xff006400;
    c.index = Palette_Nearest(palette, 0, 0x64, 0);
    Gauge_SetColour2(gauge, nullptr, c);
    Gauge_SetMarksY(gauge, nullptr, y0, y1, y2);
    Gauge_SetMarksX(gauge, nullptr, x0, x1, x2);
}

u32 __cdecl Gauge_DrawInWindow(u8 *window, u8 *gauge, void(__cdecl *after)(u8 *)) {
    SurfaceInfo info;
    SurfaceInfo_Ctor(&info);
    u32 width, height;
    const u32 bits = Window_ImplSlot7(window, nullptr, (u32)(uintptr_t)&width, (u32)(uintptr_t)&height);
    if (!bits) return 0;
    Format_FillInfo(bits, reinterpret_cast<u32 *>(&info), width, height);
    Display_SetSurfaceInfo(&info);   // (the stack info stays the display's surface, sic)
    DrawColour black = {};
    black.rgb = 0xff000000u;
    black.index = Palette_Nearest(reinterpret_cast<const u8 *>((uintptr_t)0x00fa71f8), 0, 0, 0);
    const Rect rect = {0, 0, static_cast<s32>(width), static_cast<s32>(height)};
    Display_BoxFill(&rect, black);
    Gauge_DrawBack(gauge);
    Gauge_DrawGrid(gauge);
    Gauge_DrawAxes(gauge);
    if (after) after(gauge);
    return Window_ImplSlot6(window);
}
}
