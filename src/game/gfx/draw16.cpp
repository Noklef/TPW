// The drawing primitives: see draw.h. The 16-bit table (0x703518).
//
// A colour is packed into the surface's format through the context's PixelShifts. The ghost
// flags do not blend at this depth: they only stop the remap, which (flag 0x20) otherwise takes
// the colour from the palette entry of the remapped index.
#include "draw.h"

#include "drawimpl.h"

using namespace draw_impl;

namespace {

u16 pack(const DrawContext *s, u32 rgb) {
    auto channel = [rgb](const PixelChannel &c) { return ((rgb >> (c.up & 31)) << (c.down & 31)) & c.mask; };
    const PixelShifts &p = s->shifts;
    return static_cast<u16>(channel(p.red) | channel(p.green) | channel(p.blue) | channel(p.alpha));
}

// The colour a primitive draws: remapped through the palette only without the ghost flags.
u16 colour(const DrawContext *s, DrawColour c) {
    if (!(s->flags & (kDrawGhost | kDrawGhostReverse)) && (s->flags & kDrawRemap)) {
        const u8 *e = g_env->palette + s->remap[c.index] * 4u;
        return pack(s, 0xff000000u | e[0] << 16 | e[1] << 8 | e[2]);
    }
    return pack(s, c.rgb);
}

u16 *pixel(const DrawContext *s, s32 x, s32 y) {
    return reinterpret_cast<u16 *>(s->bits + static_cast<u32>(s->surface->pitch * y + x * 2));
}

void fill(u16 *p, s32 count, u16 value) {
    for (; count > 0; --count) *p++ = value;
}

}  // namespace

extern "C" {

void __fastcall Draw16_Pixel(DrawDriver *, void *, s32 x, s32 y, DrawColour c) {
    const DrawContext *s = g_env->state;
    if (inClip(s, x, y)) *pixel(s, x, y) = colour(s, c);
}

void __fastcall Draw16_BoxFill(DrawDriver *, void *, const Rect *r, DrawColour c) {
    const DrawContext *s = g_env->state;
    Rect box;
    Rect_Clip(r, nullptr, &box, &s->clip);
    const s32 w = Rect_Width(&box), h = Rect_Height(&box);
    const u16 value = colour(s, c);
    u16 *row = pixel(s, box.left, box.top);
    for (s32 n = h; n > 0; --n, row = reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(row) + s->surface->pitch))
        fill(row, w, value);
}

void __fastcall Draw16_HLine(DrawDriver *, void *, s32 x, s32 y, s32 length, DrawColour c) {
    const DrawContext *s = g_env->state;
    if (length < 0) {
        x += length;
        length = -length;
    }
    if (s->clip.left > x + length || x > s->clip.right || s->clip.top > y || y >= s->clip.bottom) return;
    if (x < s->clip.left) {
        length = x + length - s->clip.left;
        x = s->clip.left;
    }
    if (s->clip.right < x + length) length = s->clip.right - x;
    fill(pixel(s, x, y), length, colour(s, c));
}

void __fastcall Draw16_VLine(DrawDriver *, void *, s32 x, s32 y, s32 length, DrawColour c) {
    const DrawContext *s = g_env->state;
    if (length < 0) {
        y += length;
        length = -length;
    }
    if (s->clip.top > y + length || y > s->clip.bottom || s->clip.left > x || x >= s->clip.right) return;
    if (y < s->clip.top) {
        length = y + length - s->clip.top;
        y = s->clip.top;
    }
    if (s->clip.bottom < y + length) length = s->clip.bottom - y;
    const u16 value = colour(s, c);
    u8 *p = reinterpret_cast<u8 *>(pixel(s, x, y));
    for (; length > 0; --length, p += s->surface->pitch) *reinterpret_cast<u16 *>(p) = value;
}

void __fastcall Draw16_Clear(DrawDriver *, void *, DrawColour c) {
    const DrawContext *s = g_env->state;
    const s32 w = Rect_Width(&s->clip), h = Rect_Height(&s->clip);
    u16 *row = pixel(s, s->clip.left, s->clip.top);
    for (s32 n = h; n > 0; --n, row = reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(row) + s->surface->pitch))
        fill(row, w, pack(s, c.rgb));
}

void __fastcall Draw16_Text(DrawDriver *, void *, s32 x, s32 y, const char *str, DrawColour c) {
    text<u16>(x, y, reinterpret_cast<const signed char *>(str), pack(g_env->state, c.rgb));
}

void __fastcall Draw16_TextW(DrawDriver *, void *, s32 x, s32 y, const u16 *str, DrawColour c) {
    text<u16>(x, y, str, pack(g_env->state, c.rgb));
}

}  // extern "C"
