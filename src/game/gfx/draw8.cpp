// The drawing primitives: see draw.h. The shared slots and the 8-bit table.
#include "draw.h"

#include "../core/vcall.h"

#include <cstdint>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const DrawEnv kGameEnv = {
    at<DrawContext *>(0x00fa76b8),
    at<const u8 *>(0x00778108),
};
const DrawEnv *g_env = &kGameEnv;

bool inClip(const DrawContext *s, s32 x, s32 y) {
    return s->clip.left <= x && x < s->clip.right && s->clip.top <= y && y < s->clip.bottom;
}

// The colour a plain (unblended) primitive writes.
u8 plain(const DrawContext *s, u8 index) { return s->flags & kDrawRemap ? s->remap[index] : index; }

// One pixel through the flags.
void put(const DrawContext *s, u8 *p, u8 index) {
    if (s->flags & kDrawGhost) *p = s->ghost[index * 256u + *p];
    else if (s->flags & kDrawGhostReverse) *p = s->ghost[*p * 256u + index];
    else *p = plain(s, index);
}

// A run of count bytes, dwords first (rep stosd, then the rest).
void fill(u8 *p, u32 count, u8 index) {
    const u32 quad = index * 0x01010101u;
    for (u32 n = count >> 2; n; --n, p += 4) *reinterpret_cast<u32 *>(p) = quad;
    for (u32 n = count & 3; n; --n) *p++ = index;
}

// A run of count blended pixels (count < 1: none).
void blend(const DrawContext *s, u8 *p, s32 count, u8 index, s32 step) {
    if (count - 1 < 0) return;
    if (s->flags & kDrawGhost) {
        for (; count; --count, p += step) *p = s->ghost[index * 256u + *p];
    } else {
        for (; count; --count, p += step) *p = s->ghost[*p * 256u + index];
    }
}

// The 5x6 debug font: glyph rows 0x40..0x04, the advance in byte 6.
template <typename C>
void text(s32 x, s32 y, const C *str, u8 index) {
    const DrawContext *s = g_env->state;
    if (y + 6 > s->clip.bottom || s->clip.top > y || x + 6 > s->clip.right || !str) return;
    const s32 pitch = s->surface->pitch;
    u32 cx = static_cast<u32>(x);
    for (C ch; (ch = *str++) != 0;) {
        u32 advance = 0;
        if (ch == 10) {
            if (s->clip.bottom < y + 14) return;
            y += 8;
            cx = static_cast<u32>(x);
        } else if (ch == 9) {
            cx = (cx + 0x20) & 0xffe0;
        } else {
            u32 glyph = static_cast<u32>(static_cast<s32>(ch)) - 0x20;
            if (glyph > 0x88) glyph = 0;
            const u8 *g = g_env->font + glyph * 8;
            advance = g[6];
            if (s->clip.left <= static_cast<s32>(cx) && static_cast<s32>(cx + 6) <= s->clip.right) {
                u8 *p = s->bits + static_cast<u32>(pitch * y) + cx;
                for (s32 row = 0; row < 6; ++row, p += pitch) {
                    const u8 bits = g[row];
                    if (bits & 0x40) p[0] = index;
                    if (bits & 0x20) p[1] = index;
                    if (bits & 0x10) p[2] = index;
                    if (bits & 0x08) p[3] = index;
                    if (bits & 0x04) p[4] = index;
                }
            }
        }
        cx += advance;
    }
}

}  // namespace

void Draw_SetEnv(const DrawEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void __fastcall Draw_Box(DrawDriver *d, void *, const Rect *r, DrawColour c) {
    vcall<void>(d, g_env->state->flags & kDrawOutline ? 8 : 0xc, r, c);
}

void __fastcall Draw_BoxOutline(DrawDriver *d, void *, const Rect *r, DrawColour c) {
    const s32 w = Rect_Width(r), h = Rect_Height(r);
    vcall<void>(d, 0x20, r->left, r->top, w - 1, c);
    vcall<void>(d, 0x24, r->right - 1, r->top, h - 1, c);
    vcall<void>(d, 0x20, r->left + 1, r->bottom - 1, w - 1, c);
    vcall<void>(d, 0x24, r->left, r->top + 1, h - 1, c);
}

void __fastcall Draw_Circle(DrawDriver *d, void *, s32 x, s32 y, s32 r, DrawColour c) {
    vcall<void>(d, g_env->state->flags & kDrawOutline ? 0x14 : 0x18, x, y, r, c);
}

void __fastcall Draw8_Pixel(DrawDriver *, void *, s32 x, s32 y, DrawColour c) {
    const DrawContext *s = g_env->state;
    if (!inClip(s, x, y)) return;
    put(s, s->bits + static_cast<u32>(s->surface->pitch * y) + x, c.index);
}

void __fastcall Draw8_BoxFill(DrawDriver *, void *, const Rect *r, DrawColour c) {
    const DrawContext *s = g_env->state;
    Rect box;
    Rect_Clip(r, nullptr, &box, &s->clip);
    const u32 w = static_cast<u32>(Rect_Width(&box));
    s32 h = Rect_Height(&box);
    const s32 pitch = s->surface->pitch;
    u32 offset = static_cast<u32>(pitch * box.top + box.left);
    if (!(s->flags & (kDrawGhost | kDrawGhostReverse))) {
        const u8 index = plain(s, c.index);
        for (u8 *row = s->bits + offset; h - 1 >= 0 && h; --h, row += pitch) fill(row, w, index);
        return;
    }
    if (h - 1 < 0) return;
    for (; h; --h, offset += pitch - w) {
        blend(s, s->bits + offset, static_cast<s32>(w), c.index, 1);
        if (static_cast<s32>(w) - 1 >= 0) offset += w;
    }
}

void __fastcall Draw8_HLine(DrawDriver *, void *, s32 x, s32 y, s32 length, DrawColour c) {
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
    u8 *p = s->bits + static_cast<u32>(s->surface->pitch * y + x);
    if (s->flags & (kDrawGhost | kDrawGhostReverse)) blend(s, p, length, c.index, 1);
    else fill(p, static_cast<u32>(length), plain(s, c.index));
}

void __fastcall Draw8_VLine(DrawDriver *, void *, s32 x, s32 y, s32 length, DrawColour c) {
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
    const s32 pitch = s->surface->pitch;
    u8 *p = s->bits + static_cast<u32>(pitch * y + x);
    if (s->flags & (kDrawGhost | kDrawGhostReverse)) {
        blend(s, p, length, c.index, pitch);
        return;
    }
    const u8 index = plain(s, c.index);
    if (length - 1 < 0) return;
    for (; length; --length, p += pitch) *p = index;
}

void __fastcall Draw8_Clear(DrawDriver *, void *, DrawColour c) {
    const DrawContext *s = g_env->state;
    const s32 pitch = s->surface->pitch;
    u8 *row = s->bits + static_cast<u32>(pitch * s->clip.top) + s->clip.left;
    const u32 w = static_cast<u32>(Rect_Width(&s->clip));
    s32 h = Rect_Height(&s->clip);
    if (h - 1 < 0) return;
    for (; h; --h, row += pitch) fill(row, w, c.index);
}

void __fastcall Draw8_Text(DrawDriver *, void *, s32 x, s32 y, const char *str, DrawColour c) {
    text(x, y, reinterpret_cast<const signed char *>(str), c.index);
}

void __fastcall Draw8_TextW(DrawDriver *, void *, s32 x, s32 y, const u16 *str, DrawColour c) {
    text(x, y, str, c.index);
}

}  // extern "C"
