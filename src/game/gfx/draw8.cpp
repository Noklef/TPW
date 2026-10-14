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

// a * b / c as imul, cdq, idiv: the product wraps to 32 bits.
s32 mulDiv(s32 a, s32 b, s32 c) {
    return static_cast<s32>(static_cast<u32>(a) * static_cast<u32>(b)) / c;
}

// Clip one axis of a line to [lo, hi): u is the axis clipped, v the other. False: the line is
// entirely outside. Both ends are moved by interpolation; the far end of the other axis is not
// checked again.
bool clipAxis(s32 &u1, s32 &v1, s32 &u2, s32 &v2, s32 lo, s32 hi) {
    s32 d;
    if (u1 < lo) {
        if (u2 < lo) return false;
        v1 -= mulDiv(u1 - lo, v1 - v2, u1 - u2);
        u1 = lo;
        if (u2 < hi) return true;
        d = mulDiv(u2 - hi + 1, v2 - v1, u2 - lo);
        u2 = hi - 1;
    } else if (u1 < hi) {
        if (u2 < lo) {
            d = mulDiv(u2 - lo, v2 - v1, u2 - u1);
            u2 = lo;
        } else if (hi <= u2) {
            d = mulDiv(u2 - hi + 1, v2 - v1, u2 - u1);
            u2 = hi - 1;
        } else {
            return true;
        }
    } else {
        if (hi <= u2) return false;
        v1 -= mulDiv(u1 - hi + 1, v1 - v2, u1 - u2);
        u1 = hi - 1;
        if (lo <= u2) return true;
        d = mulDiv(u2 - lo, v2 - v1, u2 - u1);
        u2 = lo;
    }
    v2 -= d;
    return true;
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

void __fastcall Draw_Triangle(DrawDriver *d, void *, s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3,
                              DrawColour c) {
    if (g_env->state->flags & kDrawOutline) {
        vcall<void>(d, 0x1c, x1, y1, x2, y2, c);
        vcall<void>(d, 0x1c, x2, y2, x3, y3, c);
        vcall<void>(d, 0x1c, x3, y3, x1, y1, c);
        return;
    }
    auto swap = [](s32 &xa, s32 &ya, s32 &xb, s32 &yb) {
        const s32 x = xa, y = ya;
        xa = xb, ya = yb, xb = x, yb = y;
    };
    if (y1 > y2) swap(x1, y1, x2, y2);
    if (y2 > y3) swap(x2, y2, x3, y3);
    if (y1 > y2) swap(x1, y1, x2, y2);
    // 16.16 edges; spans [left, right) go to the horizontal line. The long edge (1 to 3) is
    // walked throughout, the short ones (1 to 2, then 2 to 3) in turn.
    auto fixed = [](s32 v) { return static_cast<s32>(static_cast<u32>(v) << 16); };
    auto slope = [&](s32 dx, s32 dy) { return fixed(dx) / dy; };
    auto spans = [&](s32 &y, s32 to, s32 &left, s32 dl, s32 &right, s32 dr) {
        for (; y < to; ++y, left += dl, right += dr)
            vcall<void>(d, 0x20, left >> 16, y, (right >> 16) - (left >> 16), c);
    };
    s32 y = y1, left, right, dl, dr;
    if (y1 == y2) {
        if (y2 == y3) return;
        if (x1 < x2) {
            dl = slope(x3 - x1, y3 - y1);
            left = fixed(x1);
            dr = slope(x3 - x2, y3 - y2);
            right = fixed(x2);
        } else {
            dl = slope(x3 - x2, y3 - y2);
            left = fixed(x2);
            dr = slope(x3 - x1, y3 - y1);
            right = fixed(x1);
        }
        spans(y, y3, left, dl, right, dr);
        return;
    }
    const s32 dLong = slope(x3 - x1, y3 - y1), dShort = slope(x2 - x1, y2 - y1);
    s32 edge = fixed(x1), other = fixed(x1);
    if (dShort <= dLong) {
        spans(y, y2, other, dShort, edge, dLong);
        if (y2 == y3) return;
        other = fixed(x2);
        spans(y, y3, other, slope(x3 - x2, y3 - y2), edge, dLong);
    } else {
        spans(y, y2, edge, dLong, other, dShort);
        if (y2 == y3) return;
        other = fixed(x2);
        spans(y, y3, edge, dLong, other, slope(x3 - x2, y3 - y2));
    }
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

void __fastcall Draw8_Line(DrawDriver *d, void *, s32 x1, s32 y1, s32 x2, s32 y2, DrawColour c) {
    if (x1 == x2) {
        vcall<void>(d, 0x24, x1, y1, y2 - y1, c);
        return;
    }
    if (y1 == y2) {
        vcall<void>(d, 0x20, x1, y1, x2 - x1, c);
        return;
    }
    const DrawContext *s = g_env->state;
    if (!clipAxis(x1, y1, x2, y2, s->clip.left, s->clip.right)) return;
    if (!clipAxis(y1, x1, y2, x2, s->clip.top, s->clip.bottom)) return;
    // Bresenham, stepping along the longer axis (y on a tie).
    const s32 dx = x2 - x1, dy = y2 - y1;
    const s32 ddx = 2 * (dx < 0 ? -dx : dx), ddy = 2 * (dy < 0 ? -dy : dy);
    const s32 sx = dx < 1 ? -1 : 1, sy = dy < 1 ? -1 : 1;
    const s32 pitch = s->surface->pitch, stepY = sy * pitch;
    s32 offset = y1 * pitch + x1;
    const bool blended = (s->flags & (kDrawGhost | kDrawGhostReverse)) != 0;
    const u8 index = blended ? c.index : plain(s, c.index);
    auto draw = [&] {
        u8 *p = s->bits + offset;
        if (s->flags & kDrawGhost) *p = s->ghost[index * 256u + *p];
        else if (s->flags & kDrawGhostReverse) *p = s->ghost[*p * 256u + index];
        else *p = index;
    };
    draw();
    if (ddx <= ddy) {
        for (s32 err = -(ddy >> 1); y1 != y2; y1 += sy) {
            err += ddx;
            if (err >= 0) {
                offset += sx;
                err -= ddy;
            }
            offset += stepY;
            draw();
        }
    } else {
        for (s32 err = -(ddx >> 1); x1 != x2; x1 += sx) {
            err += ddy;
            if (err >= 0) {
                offset += stepY;
                err -= ddx;
            }
            offset += sx;
            draw();
        }
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
