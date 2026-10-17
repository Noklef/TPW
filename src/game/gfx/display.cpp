// The drawing front end: see display.h.
#include "display.h"

#include <cstdint>

#include "../core/vcall.h"

namespace {

DrawContext *const kGameState = reinterpret_cast<DrawContext *>(static_cast<std::uintptr_t>(0x00fa76b8));
DrawContext *g_state = kGameState;

s32 lesser(s32 a, s32 b) { return a < b ? a : b; }
s32 greater(s32 a, s32 b) { return a > b ? a : b; }

void selectDriver(DrawContext *s) {
    switch (s->surface->bitCount) {
    case 16: s->driver = &s->driver16; break;
    case 24: s->driver = &s->driver24; break;
    case 32: s->driver = &s->driver32; break;
    default: s->driver = &s->driver8; break;
    }
    PixelShifts_Set(&s->shifts, nullptr, &s->surface->redMask);
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

void Display_SetEnv(DrawContext *state) { g_state = state ? state : kGameState; }

extern "C" {

u32 __cdecl Display_IsVisible(const Rect *r) {
    Rect box = *r;
    Rect_Order(&box);
    return Rect_Overlaps(&g_state->clip, nullptr, &box);
}

void __cdecl Display_Pixel(s32 x, s32 y, DrawColour c) { vcall<void>(g_state->driver, 0, x, y, c); }

void __cdecl Display_Line(s32 x1, s32 y1, s32 x2, s32 y2, DrawColour c) {
    const Rect box = {x1, y1, x2, y2};
    if (Display_IsVisible(&box)) vcall<void>(g_state->driver, 0x1c, x1, y1, x2, y2, c);
}

void __cdecl Display_HLine(s32 x, s32 y, s32 length, DrawColour c) {
    const Rect box = {x, y, x + length, y + 1};
    if (Display_IsVisible(&box)) vcall<void>(g_state->driver, 0x20, x, y, length, c);
}

void __cdecl Display_VLine(s32 x, s32 y, s32 length, DrawColour c) {
    const Rect box = {x, y, x + 1, y + length};
    if (Display_IsVisible(&box)) vcall<void>(g_state->driver, 0x24, x, y, length, c);
}

void __cdecl Display_Triangle(s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, DrawColour c) {
    const Rect box = {lesser(x1, lesser(x2, x3)), lesser(y1, lesser(y2, y3)), greater(x1, greater(x2, x3)),
                      greater(y1, greater(y2, y3))};
    if (Display_IsVisible(&box)) vcall<void>(g_state->driver, 0x28, x1, y1, x2, y2, x3, y3, c);
}

void __cdecl Display_FillRect(const Rect *r, DrawColour c) { Display_BoxFill(r ? r : &g_state->clip, c); }

void __cdecl Display_BoxFill(const Rect *r, DrawColour c) {
    if (Display_IsVisible(r)) vcall<void>(g_state->driver, 0xc, r, c);
}

void __cdecl Display_SetWindow(const Rect *r) {
    DrawContext *s = g_state;
    if (r) {
        s->window = *r;
    } else {
        s->window = {0, 0, static_cast<s32>(s->surface->width), static_cast<s32>(s->surface->height)};
    }
    Rect_Normalize(&s->window);
    const SurfaceInfo *i = s->surface;
    const u32 offset = (i->bitCount >> 3) * static_cast<u32>(s->window.left) +
                       static_cast<u32>(i->pitch) * static_cast<u32>(s->window.top);
    s->bits = static_cast<u8 *>(i->bits) + offset;
    Display_SetClip(nullptr);
}

void __cdecl Display_SetClip(const Rect *r) {
    DrawContext *s = g_state;
    const Rect &w = s->window;
    Rect part = {0, 0, w.right - w.left, w.bottom - w.top};
    if (r) {
        const Rect all = part;
        part = *r;
        Rect_Normalize(&part);
        if (!Rect_Intersect(&part, &all)) Rect_Collapse(&part);
    }
    s->clip = {-w.left, -w.top, static_cast<s32>(s->surface->width - static_cast<u32>(w.left)),
               static_cast<s32>(s->surface->height - static_cast<u32>(w.top))};
    if (!Rect_Intersect(&s->clip, &part)) s->clip = {0, 0, 0, 0};
}

Rect *__cdecl Display_GetClip() { return &g_state->clip; }

void __cdecl Display_SetSurfaceInfo(SurfaceInfo *i) {
    DrawContext *s = g_state;
    s->surface = i;
    if (!i) return;
    s->bits = static_cast<u8 *>(i->bits);
    Display_SetWindow(nullptr);
    selectDriver(s);
}

void __cdecl Display_SetSurface(Surface *surface) {
    DrawContext *s = g_state;
    if (!surface) {
        s->surface = nullptr;
        return;
    }
    s->surface = Surface_Refresh(surface);
    s->bits = static_cast<u8 *>(s->surface->bits);
    s->flags = surface->drawFlags;
    s->ghost = surface->ghost;
    s->unknown34 = surface->unknown08;
    s->remap = surface->remap;
    Display_SetWindow(nullptr);
    selectDriver(s);
}

void __cdecl Format_FillInfo(u32 a, u32 *out, u32 c, u32 d) {
    if (!out)
        return;
    const u32 *record = reinterpret_cast<const u32 *>((uintptr_t)(0x008bd558 + global<u32>(0x008bd554) * 32));
    u32 local[8];
    for (u32 i = 0; i < 8; ++i)
        local[i] = record[i];
    out[6] = a;
    out[7] = c;
    out[4] = local[3];
    out[8] = d;
    out[0] = local[4];
    out[9] = (local[3] >> 3) * c;
    out[1] = local[5];
    out[2] = local[6];
    out[3] = local[7];
    out[5] = local[1] & 0x20;
}
}  // extern "C"
