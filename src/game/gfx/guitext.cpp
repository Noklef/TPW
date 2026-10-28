#include "guitext.h"

#include <cstdint>

#include "../core/language.h"
#include "../core/methods.h"
#include "../core/string.h"
#include "../math/rect.h"
#include "display.h"
#include "palette.h"
#include "textlayout.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *const kStyle = reinterpret_cast<u8 *>(0x0078ba2c);

// The common part: the layout's kinds from the alignments, the rectangle (widened without fit), the global style's font. Returns the
// rectangle to lay the text out in.
Rect Prepare(u8 *layout, const s32 *rect, s8 font, s8 hAlign, s8 vAlign, u32 fit) {
    u32 left = (u32)rect[0];
    const u32 top = (u32)rect[1];
    const u32 width = (u32)rect[2] - (u32)rect[0];
    const u32 height = (u32)rect[3] - (u32)rect[1];
    u32 shown = width;
    s32 h = hAlign;
    if (Language_IsHebrew(reinterpret_cast<const u8 *>(0x0078b530))) {
        if (h == 0)
            h = 2;
        else if (h == 2)
            h = 0;
    }
    u32 status;
    switch (h) {
    case 0:
        Obj_SetKindA4(layout, nullptr, &status, 0);
        if (!fit) shown = 0x4000;
        break;
    case 1:
        Obj_SetKindA4(layout, nullptr, &status, 2);
        if (!fit) {
            left = (u32)(((s32)width >> 1) - 0x2000);  // sic: the rectangle's left dropped
            shown = 0x4000;
        }
        break;
    case 2:
        Obj_SetKindA4(layout, nullptr, &status, 1);
        if (!fit) {
            shown = 0x4000;
            left = width - 0x4000;  // sic: likewise
        }
        break;
    }
    switch (vAlign) {
    case 0:
        Obj_SetKindA0(layout, nullptr, &status, 4);
        break;
    case 1:
        Obj_SetKindA0(layout, nullptr, &status, 2);
        break;
    case 2:
        Obj_SetKindA0(layout, nullptr, &status, 5);
        break;
    }
    const u32 index = (u32)(s32)font;
    const uintptr_t entry = index < 0xd ? 0x0078b54c + (index + global<u32>(0x0078b534) * 13) * 24 : 0;  // (none: a read of 0x14, sic)
    s32 set;
    TextStyle_SetFont(kStyle, nullptr, &set, *reinterpret_cast<u8 *const *>(entry + 0x14));
    Rect r;
    r.left = (s32)left;
    r.top = (s32)top;
    r.right = (s32)(left + shown);
    r.bottom = (s32)(top + height);
    return r;
}
}  // namespace

extern "C" {

void __cdecl GuiText_Draw(const u16 *text, const s32 *rect, const u8 *colour, s8 font, s8 hAlign, s8 vAlign, u32 fit, s32 *bounds,
                          u32 keepSurface) {
    alignas(4) u8 layout[0xc0];
    TextLayout_Ctor(layout);
    s32 box[4] = {0, 0, 0, 0};
    if ((global<u32>(0x007ca120) || keepSurface) && text) {
        Rect r = Prepare(layout, rect, font, hAlign, vAlign, fit);
        s32 status;
        TextStyle_SetColour(kStyle, nullptr, &status, colour);
        if (!keepSurface) Display_SetSurfaceInfo(reinterpret_cast<SurfaceInfo *>(0x007ca0e8));
        Rect clip = r;  // (the original sets its rectangle again after the clip)
        Display_SetClip(&clip);
        const u8 *narrow = reinterpret_cast<const u8 *>((uintptr_t)Text_NarrowToGlobalBuffer(text));
        TextLayout_Draw(layout, nullptr, &status, &r.left, narrow, kStyle, box);
        Display_SetClip(nullptr);
        if (bounds)
            for (u32 i = 0; i < 4; ++i) bounds[i] = box[i];
    }
    TextLayout_Dtor(layout);
}

void __cdecl GuiText_Measure(const u16 *text, const s32 *rect, s8 font, s8 hAlign, s8 vAlign, u32 fit, s32 *box) {
    alignas(4) u8 layout[0xc0];
    TextLayout_Ctor(layout);
    s32 measured[4] = {0, 0, 0, 0};
    if (text) {
        Rect r = Prepare(layout, rect, font, hAlign, vAlign, fit);
        const u8 *narrow = reinterpret_cast<const u8 *>((uintptr_t)Text_NarrowToGlobalBuffer(text));
        s32 status;
        TextLayout_Measure(layout, nullptr, &status, &r.left, narrow, kStyle, measured);
        if (box)
            for (u32 i = 0; i < 4; ++i) box[i] = measured[i];
    }
    TextLayout_Dtor(layout);
}

void __cdecl GuiText_DrawRgb(const u16 *text, u32 a, u32 c, u32 d, s32 left, s32 top, s32 right, s32 bottom, u32 rgb) {
    u8 colour[8] = {};   // blue, green, red, 0xff, the palette index (the original's last three bytes are never read)
    colour[0] = (u8)rgb;
    colour[1] = (u8)(rgb >> 8);
    colour[2] = (u8)(rgb >> 16);
    colour[3] = 0xff;
    colour[4] = Palette_Nearest(reinterpret_cast<const u8 *>(0x00fa71f8), rgb >> 16, rgb >> 8, rgb);
    Format_FillInfoByKind(a, c, d, 3, reinterpret_cast<u32 *>(0x007ca0e8));
    global<u32>(0x007ca120) = 1;
    u32 status;
    Obj_SetOn10(kStyle, nullptr, &status);
    const s32 rect[4] = {left, top, right, bottom};
    GuiText_Draw(text, rect, colour, 0xc, 0, 0, 1, nullptr, 0);
    Obj_SetOff10(kStyle, nullptr, &status);
    global<u32>(0x007ca120) = 0;
}
}
