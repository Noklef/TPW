#include "textparts.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "../core/language.h"
#include "../core/methods.h"
#include "../core/seqs.h"
#include "../core/string.h"
#include "../core/textarg.h"
#include "../core/vcall.h"
#include "../gfx/display.h"
#include "../gfx/palette.h"
#include "../gfx/spriteframe.h"
#include "../gfx/surface.h"
#include "../gfx/textlayout.h"
#include "../math/x87.h"
#include "renderer.h"

namespace {

const u32 kInv255 = 0x3b808081, k255 = 0x437f0000;

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *Construct(u8 *part, u32 value, u32 vtable, bool layoutFlag) {
    RendererPart_Ctor(part, nullptr, value);
    TextLayout_Ctor(part + 0xe4);
    at<u8>(part, 0x48) = 0xff;
    at<u8>(part, 0x49) = 0xff;
    at<u8>(part, 0x4a) = 0xff;
    if (layoutFlag) at<u8>(part, 0x178) = 1;
    at<u8>(part, 0xd8) = 0;
    at<u8>(part, 0xd9) = 1;
    at<u8>(part, 0xda) = 0;
    at<u8>(part, 0xdb) = 0;
    at<u16>(part, 0xde) = 0;
    at<u16>(part, 0xe0) = 0;
    at<u8>(part, 0xdc) = 0xff;
    at<u8>(part, 0xdd) = 0;
    at<u32>(part, 0) = vtable;
    return part;
}

// a colour byte through the float constants 0x6ff2bc (1 / 255) and 0x6ff2c0 (255), at the x87's precision; __ftol's low half
u32 ColourByte(u8 c) {
    const X87Reg x = X87Reg((double)c) * X87Reg((double)reinterpret_cast<const float &>(kInv255)) * X87Reg((double)reinterpret_cast<const float &>(k255));
    return (u32)(long long)x.v;
}

void Destroy(u8 *part) {
    at<u32>(part, 0) = 0x006ff320;
    TextLayout_Dtor(part + 0xe4);
    RendererPart_Dtor(part);
}
}

extern "C" {
u8 *__fastcall TextPart_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff320, false); }

void __fastcall TextPart_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff350_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff350, false); }

void __fastcall TextPart6ff350_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff350_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff350_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff380_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff380, true); }

void __fastcall TextPart6ff380_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff380_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff380_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff3b0_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff3b0, false); }

void __fastcall TextPart6ff3b0_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff3b0_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff3b0_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff3e0_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff3e0, false); }

void __fastcall TextPart6ff3e0_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff3e0_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff3e0_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff410_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff410, false); }

void __fastcall TextPart6ff410_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff410_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff410_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff440_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff440, false); }

void __fastcall TextPart6ff440_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff440_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff440_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff470_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff470, false); }

void __fastcall TextPart6ff470_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff470_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff470_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff4a0_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff4a0, false); }

void __fastcall TextPart6ff4a0_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff4a0_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff4a0_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff4d0_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff4d0, true); }

void __fastcall TextPart6ff4d0_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff4d0_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff4d0_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff500_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff500, false); }

void __fastcall TextPart6ff500_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff500_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff500_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

u8 *__fastcall TextPart6ff530_Ctor(u8 *part, void *, u32 value) { return Construct(part, value, 0x006ff530, false); }

void __fastcall TextPart6ff530_Dtor(u8 *part) { Destroy(part); }

u8 *__fastcall TextPart6ff530_DeletingDtor(u8 *part, void *, u32 flags) {
    TextPart6ff530_Dtor(part);
    if (flags & 1) GameHeap_Free(part);
    return part;
}

void __fastcall TextPart_SetLayout(u8 *part, void *, u32 flags, const u8 *info) {
    if (!info || !flags) return;
    RendererPart_SetLayout(part, nullptr, flags, info);
    if (!(flags & 0x7fd00)) return;
    const u8 *const text = at<const u8 *>(const_cast<u8 *>(info), 0x10);
    if (!text) return;
    if (flags & 0x400) at<u8>(part, 0xd8) = text[4];
    if (flags & 0x800) at<u8>(part, 0xd9) = text[5];
    if (flags & 0x1000) at<u8>(part, 0xda) = text[6];
    if (flags & 0x2000) at<u8>(part, 0xdb) = text[7];
    if (flags & 0x4000) at<u16>(part, 0xde) = *reinterpret_cast<const u16 *>(text + 0xa);
    if (flags & 0x8000) at<u16>(part, 0xe0) = *reinterpret_cast<const u16 *>(text + 0xc);
    if (flags & 0x10000) at<u8>(part, 0xdc) = text[8];
    if (flags & 0x20000) at<u8>(part, 0xdd) = text[9];
    if (flags & 0x40000) {   // (the colour bytes +0x48..+0x4b, then +0x78 = +0x4b, +0x48, +0x49, +0x4a from the top, each through the floats)
        at<u32>(part, 0x48) = *reinterpret_cast<const u32 *>(text + 0x10);
        u32 colour = ColourByte(at<u8>(part, 0x4b));
        colour = colour << 8 | ColourByte(at<u8>(part, 0x48));
        colour = colour << 8 | ColourByte(at<u8>(part, 0x49));
        colour = colour << 8 | ColourByte(at<u8>(part, 0x4a));
        at<u32>(part, 0x78) = colour;
    }
    if (flags & 0x100) {
        const u32 object = *reinterpret_cast<const u32 *>(text);
        if (object) vcall<void>(part, 0x24, object);   // (slot 9; 0x48f830 in this class)
    }
}

namespace {

// The number classes' SetLayouts: the text id's text with the number argument n (value, minus flag) into 0x3f characters on the stack.
void format_number(u16 *text, u32 id, u32 n, u32 value, u32 minus) {
    u32 number[5] = {0x006ff170, 0x5641524d, n, value, minus};
    TextFormat().format(*reinterpret_cast<void **>((uintptr_t)0x0078ba94), text, 0x3f, id, number);
}

// ... then that text sent through SetLayout's flag 0x100 (the info's +0x10 a record whose +0 is the text; nothing else of either read).
void set_text(u8 *part, u16 *text) {
    u32 record[5] = {(u32)(uintptr_t)text, 0, 0, 0, 0};
    u32 info[5] = {0, 0, 0, 0, (u32)(uintptr_t)record};
    TextPart_SetLayout(part, nullptr, 0x100, reinterpret_cast<const u8 *>(info));
}

u32 magnitude(s32 value) { return value < 0 ? 0u - (u32)value : (u32)value; }

u8 *const kStyle = reinterpret_cast<u8 *>(0x0078ba2c);

s32 screen_width() { return *reinterpret_cast<const s16 *>(0x00faa5c4); }
s32 screen_height() { return *reinterpret_cast<const s16 *>(0x00faa5c0); }


struct TextRect {
    s32 left, top, right, bottom;
};

// The text parts' common start (0x48f830, 0x491300, 0x491ab0): the layout (+0xe4) aligned by +0xd8 (Language_MirrorAlign) and +0xd9,
// the rectangle from the sizes +0x3c / +0x3e, the offsets +0xde / +0xe0 scaled by the screen and +0xda (0: 0x4000 wide); the global
// style given the font table's font +0xdc. Past the alignments 0-2 the left and right are the text's address (the bottom too with
// textBottom) and the original's top is the stack's.
TextRect lay_out(u8 *part, const u16 *text, bool textBottom) {
    s32 bottom = at<s16>(part, 0x3e);
    u32 align = (u32)(s32)at<s8>(part, 0xd8);
    s32 width = at<s16>(part, 0x3c);
    Language_MirrorAlign(&align);
    u8 *const layout = part + 0xe4;
    u32 status;
    s32 left, top, right, base = 0;
    switch (align) {
    case 0:
        Obj_SetKindA4(layout, nullptr, &status, 0);
        if (!at<u8>(part, 0xda)) width = 0x4000;
        left = at<s16>(part, 0xde) * screen_width() / 2048;
        right = width;
        top = at<s16>(part, 0xe0) * screen_height() / 1536;
        break;
    case 1:
        Obj_SetKindA4(layout, nullptr, &status, 2);
        if (!at<u8>(part, 0xda)) {
            base = (at<s16>(part, 0x3c) >> 1) - 0x2000;
            width = 0x4000;
        }
        left = at<s16>(part, 0xde) * screen_width() / 2048 + base;
        right = base + width;
        top = at<s16>(part, 0xe0) * screen_height() / 1536;
        break;
    case 2:
        Obj_SetKindA4(layout, nullptr, &status, 1);
        if (!at<u8>(part, 0xda)) {
            width = 0x4000;
            base = at<s16>(part, 0x3c) - 0x4000;
        }
        left = base;
        right = width - at<s16>(part, 0xde) * screen_width() / 2048 + base;
        top = 0;
        bottom -= at<s16>(part, 0xe0) * screen_height() / 1536;
        break;
    default:
        left = right = (s32)(uintptr_t)text;
        if (textBottom) bottom = left;
        top = 0;
        break;
    }
    switch (at<s8>(part, 0xd9)) {
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
    const u32 index = (u32)(s32)at<s8>(part, 0xdc);
    const uintptr_t entry = index < 0xd ? 0x0078b54c + (index + *reinterpret_cast<const u32 *>(0x0078b534) * 13) * 24 : 0;  // (none: a read of 0x14)
    s32 set;
    TextStyle_SetFont(kStyle, nullptr, &set, *reinterpret_cast<u8 *const *>(entry + 0x14));
    return TextRect{left, top, right, bottom};
}

void set_colour(u8 blue, u8 green, u8 red, u8 alpha, u8 index) {   // (the original's last three bytes never read)
    const u8 colour[8] = {blue, green, red, alpha, index, 0, 0, 0};
    s32 set;
    TextStyle_SetColour(kStyle, nullptr, &set, colour);
}

// The part's pixels (+0xa0, +0xa8 x +0xaa, the current format) the display's surface, cleared unless +0x4c answers nonzero.
void use_pixels(u8 *part, SurfaceInfo *info) {
    Format_FillInfo(at<u32>(part, 0xa0), reinterpret_cast<u32 *>(info), at<u16>(part, 0xa8), at<u16>(part, 0xaa));
    Display_SetSurfaceInfo(info);
    u32 status;
    Obj_SetOff10(kStyle, nullptr, &status);
    u8 *const pixels = at<u8 *>(part, 0xa0);
    u32(__cdecl *const before)(SurfaceInfo *) = at<u32(__cdecl *)(SurfaceInfo *)>(part, 0x4c);
    if (pixels && (!before || !before(info)))
        memset(pixels, 0, ((info->bitCount + 7) >> 3) * at<u16>(part, 0xaa) * at<u16>(part, 0xa8));
}

void draw(u8 *part, const TextRect &r, const u16 *text, s32 *box) {
    const s32 rect[4] = {r.left, r.top, r.right, r.bottom};
    const u8 *const narrow = reinterpret_cast<const u8 *>((uintptr_t)Text_NarrowToGlobalBuffer(text));
    s32 status;
    TextLayout_Draw(part + 0xe4, nullptr, &status, rect, narrow, kStyle, box);
}

// The drawn box scaled back by the screen onto +0x18 / +0x1a into the rectangle +0x24 (Fn_659dac on +0x20); +0x50 called with the
// surface; the image's (+0x9c) textures freed.
void finish(u8 *part, const s32 *box, SurfaceInfo *info) {
    const s32 gw = screen_width(), gh = screen_height();
    const u32 x = at<u16>(part, 0x18), y = at<u16>(part, 0x1a);   // (only the low words reach the rectangle)
    Fn_659dac(part + 0x20, nullptr, (u32)((s32)((u32)box[0] << 11) / gw) + x, (u32)((s32)((u32)box[1] * 0x600) / gh) + y,
              (u32)((s32)((u32)box[2] << 11) / gw) + x, (u32)((s32)((u32)box[3] * 0x600) / gh) + y);
    if (void(__cdecl *const after)(SurfaceInfo *) = at<void(__cdecl *)(SurfaceInfo *)>(part, 0x50)) after(info);
    SpriteImage_FreeTexturesIfSet(part + 0x9c);
}

}  // namespace

void __fastcall TextPart_DrawText(u8 *part, void *, const u16 *text) {
    SurfaceInfo info;
    SurfaceInfo_Ctor(&info);
    s32 box[4] = {0, 0, 0, 0};
    if (!text) return;
    const TextRect r = lay_out(part, text, false);
    if (!at<u8>(part, 0xdd)) set_colour(0xff, 0xff, 0xff, 0xff, Palette_Nearest(reinterpret_cast<const u8 *>(0x00fa71f8), 0xff, 0xff, 0xff));
    use_pixels(part, &info);
    draw(part, r, text, box);
    finish(part, box, &info);
}

void __fastcall TextPart_DrawShadowed(u8 *part, void *, const u16 *text) {
    SurfaceInfo info;
    SurfaceInfo_Ctor(&info);
    s32 box[4] = {0, 0, 0, 0};
    if (!text) return;
    const TextRect r = lay_out(part, text, false);
    use_pixels(part, &info);
    set_colour(0x80, 0x80, 0x80, 0x80, 0);
    draw(part, TextRect{r.left - 2, r.top + 2, r.right - 2, r.bottom + 2}, text, box);
    set_colour(0xff, 0xff, 0xff, 0xff, 0);
    draw(part, r, text, box);
    finish(part, box, &info);
}

u8 *__fastcall TextPart_Query(u8 *part, void *, u32 kind, u32 arg) {
    u8 *const rect = reinterpret_cast<u8 *>(0x007ca0a0);
    if (kind == 1) {
        const s32 n = (s32)arg;
        s32 x, y;
        const s32 *positions = nullptr;
        if (n >= 0) {
            positions = reinterpret_cast<const s32 *>((uintptr_t)Get88_6adf80(part + 0xe4));
            const s32 count = Vector8_Count88(part + 0xe4);
            if (!count || n >= count) positions = nullptr;
        }
        if (positions) {
            y = (s32)((u32)positions[2 * n + 1] * 0x600) / screen_height();
            x = positions[2 * n];
        } else {
            x = at<s32>(part, 0x17c);
            y = (s32)((u32)at<s32>(part, 0x180) * 0x600) / screen_height();
        }
        Fn_659dac(rect, nullptr, (u32)((s32)((u32)x << 11) / screen_width()), (u32)y, 0, 0);
        return rect;
    }
    if (kind != 2 || !arg) return nullptr;
    const u16 *const text = reinterpret_cast<const u16 *>((uintptr_t)arg);
    const TextRect r = lay_out(part, text, true);
    switch (at<s8>(part, 0xdd)) {
    case 0:
        set_colour(at<u8>(part, 0x4a), at<u8>(part, 0x49), at<u8>(part, 0x48), 0xff, 0);
        break;
    case 1: {
        u32 status;
        Obj_ResetC(kStyle, nullptr, &status);
        break;
    }
    }
    s32 box[4] = {0, 0, 0, 0};
    const s32 bounds[4] = {r.left, r.top, r.right, r.bottom};
    const u8 *const narrow = reinterpret_cast<const u8 *>((uintptr_t)Text_NarrowToGlobalBuffer(text));
    s32 status;
    TextLayout_Measure(part + 0xe4, nullptr, &status, bounds, narrow, kStyle, box);
    const s32 gw = screen_width(), gh = screen_height();
    Fn_659dac(rect, nullptr, (u32)((s32)((u32)box[0] << 11) / gw), (u32)((s32)((u32)box[1] * 0x600) / gh), (u32)((s32)((u32)box[2] << 11) / gw),
              (u32)((s32)((u32)box[3] * 0x600) / gh));
    if (*reinterpret_cast<const s16 *>(0x007ca0aa) > at<s16>(part, 0x1e) - at<s16>(part, 0x1a))
        *reinterpret_cast<u16 *>(0x007ca0aa) = (u16)(at<u16>(part, 0x1e) - at<u16>(part, 0x1a));
    return rect;
}


void __fastcall TextPart6ff3b0_SetLayout(u8 *part, void *, u32 flags, const u8 *info) {
    if (!(flags & 0x80)) return TextPart_SetLayout(part, nullptr, flags, info);
    const s32 value = *reinterpret_cast<const s32 *>(info);
    u16 text[0x40];
    format_number(text, value < 0 ? 0x1c2 : 0x1c1, value < 0 ? 0x13 : 0xd, magnitude(value), 0);
    set_text(part, text);
}

void __fastcall TextPart6ff3e0_SetLayout(u8 *part, void *, u32 flags, const u8 *info) {
    if (!(flags & 0x80)) return TextPart_SetLayout(part, nullptr, flags, info);
    const s32 value = *reinterpret_cast<const s32 *>(info);
    u16 text[0x40];
    if ((s32)magnitude(value) > 0xeeeeeee) {   // (signed: 0x80000000 stays below)
        if (value < 0) format_number(text, 0x1c2, 0x13, magnitude((s32)((u32)value + 0xeeeeeee)), 0);
        else format_number(text, 0x1c1, 0xd, (u32)value - 0xeeeeeee, 0);
    } else if (value < 0) {
        format_number(text, 0x1c7, 0x17, magnitude(value), 0);
    } else {
        format_number(text, 0x1c1, 0xd, (u32)value, 0);
    }
    set_text(part, text);
}

void __fastcall TextPart6ff410_SetLayout(u8 *part, void *, u32 flags, const u8 *info) {
    if (!(flags & 0x80)) return TextPart_SetLayout(part, nullptr, flags, info);
    const s32 value = *reinterpret_cast<const s32 *>(info);
    u16 text[0x40];
    if (!value) text[0] = 0;
    else format_number(text, 0x1c1, 0xd, (u32)value, 0);
    set_text(part, text);
}

void __fastcall TextPart6ff440_SetLayout(u8 *part, void *, u32 flags, const u8 *info) {
    if (!(flags & 0x80)) return TextPart_SetLayout(part, nullptr, flags, info);
    u16 text[0x40];
    format_number(text, 0x1c3, 0x14, *reinterpret_cast<const u32 *>(info), 0);
    set_text(part, text);
}

void __fastcall TextPart6ff4a0_SetLayout(u8 *part, void *, u32 flags, const u8 *info) {
    if (!(flags & 0x80)) return TextPart_SetLayout(part, nullptr, flags, info);
    const s32 value = *reinterpret_cast<const s32 *>(info);
    u16 text[0x40];
    if (value > 0x30) format_number(text, 0x1bb, 8, (u32)value / 24, 0);
    else if (value > 0x18) format_number(text, 0x1ba, 8, (u32)value / 24, 0);
    else format_number(text, 0x1b9, 0xb, magnitude(value), value < 0 ? 1 : 0);
    set_text(part, text);
}
}
