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
    at<SpriteDrawParams *>(0x00faa4d8),
    at<void(__cdecl *)(s32, s32, const RleSprite *)>(0x006e73c0),
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

// One pixel at (x, y) if it is inside the clip rectangle.
void plot(const DrawContext *s, s32 x, s32 y, u8 index) {
    if (inClip(s, x, y)) put(s, s->bits + static_cast<u32>(s->surface->pitch * y) + x, index);
}

// Radius 0 and 1, the same for both circles: the centre, or a plus sign drawn left, top, right,
// bottom, then the centre. False: another radius.
bool smallCircle(s32 x, s32 y, s32 r, u8 index) {
    const DrawContext *s = g_env->state;
    if (r == 0) {
        plot(s, x, y, index);
        return true;
    }
    if (r != 1) return false;
    plot(s, x - 1, y, index);
    plot(s, x, y - 1, index);
    plot(s, x + 1, y, index);
    plot(s, x, y + 1, index);
    plot(s, x, y, index);
    return true;
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

// Past the end of one run-length row.
const s8 *skipRow(const s8 *data) {
    for (s8 n; (n = *data) != 0;) data += n > 0 ? n + 1 : 1;
    return data + 1;
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
    plot(g_env->state, x, y, c.index);
}

void __fastcall Draw8_CircleOutline(DrawDriver *, void *, s32 x, s32 y, s32 r, DrawColour c) {
    if (smallCircle(x, y, r, c.index)) return;
    const DrawContext *s = g_env->state;
    // The midpoint circle, eight octants a step; points on the axes and diagonals are drawn
    // more than once (which shows when blending).
    auto octants = [&](s32 i, s32 j) {
        plot(s, x - i, y - j, c.index);
        plot(s, x + i, y - j, c.index);
        plot(s, x - i, y + j, c.index);
        plot(s, x + i, y + j, c.index);
        plot(s, x - j, y - i, c.index);
        plot(s, x + j, y - i, c.index);
        plot(s, x - j, y + i, c.index);
        plot(s, x + j, y + i, c.index);
    };
    s32 i = 0, e = 3 - 2 * r;
    if (r > 0) {
        do {
            octants(i, r);
            if (e < 0) {
                e += 6 + 4 * i;
            } else {
                e += 10 + 4 * (i - r);
                --r;
            }
            ++i;
        } while (i < r);
    }
    if (i == r) octants(i, r);
}

void __fastcall Draw8_CircleFill(DrawDriver *d, void *, s32 x, s32 y, s32 r, DrawColour c) {
    if (smallCircle(x, y, r, c.index)) return;
    // The midpoint circle again, as mirrored spans: the centre row first, then a pair of rows
    // each step, and the top and bottom rows whenever the radius steps in. A negative radius
    // comes here too and draws the centre row leftwards and two dots.
    auto span = [&](s32 left, s32 row, s32 length) { vcall<void>(d, 0x20, left, row, length, c); };
    s32 e = 3 - 2 * r;
    span(x - r, y, 2 * r + 1);
    if (e < 0) {
        e += 6;
    } else {
        span(x, y - r, 1);
        span(x, y + r, 1);
        e += 10 - 4 * r;
        --r;
    }
    s32 i = 1;
    if (r > 1) {
        do {
            span(x - r, y - i, 2 * r + 1);
            span(x - r, y + i, 2 * r + 1);
            if (e < 0) {
                e += 6 + 4 * i;
            } else {
                span(x - i, y - r, 2 * i + 1);
                span(x - i, y + r, 2 * i + 1);
                e += 10 + 4 * (i - r);
                --r;
            }
            ++i;
        } while (i < r);
    }
    if (i == r) {
        span(x - r, y - i, 2 * r + 1);
        span(x - r, y + i, 2 * r + 1);
    }
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

void __fastcall Draw8_Sprite(DrawDriver *, void *, s32 x, s32 y, const RleSprite *sprite) {
    const DrawContext *s = g_env->state;
    SpriteDrawParams *p = g_env->spriteParams;
    p->pitch = s->surface->pitch;
    p->clipWidth = Rect_Width(&s->clip);
    p->clipHeight = Rect_Height(&s->clip);
    p->flags = s->flags;
    p->ghost = s->ghost;
    p->bits = s->bits;
    p->clipTop = s->clip.top;
    p->clipLeft = s->clip.left;
    g_env->drawSprite(x - s->clip.left, y - s->clip.top, sprite);
}

// The sprite's shape in one colour: every opaque pixel is filled (no remap) or blended. The
// visible rows and columns are counted in bytes, so a sprite with 256 or more of either
// wraps: a multiple of 256 visible rows runs on through memory, as in the original.
void __fastcall Draw8_SpriteShape(DrawDriver *, void *, s32 x, s32 y, const RleSprite *sprite, DrawColour c) {
    const DrawContext *s = g_env->state;
    const Rect &clip = s->clip;
    const s32 w = sprite->width, h = sprite->height;
    if (x + w <= clip.left || x >= clip.right || y + h <= clip.top || y >= clip.bottom) return;
    const s32 skipLeft = x < clip.left ? clip.left - x : 0;
    const s32 right = x + w > clip.right ? clip.right - x : w;
    const s32 skipTop = y < clip.top ? clip.top - y : 0;
    const s32 bottom = y + h > clip.bottom ? clip.bottom - y : h;
    const s32 pitch = s->surface->pitch;
    u8 *p;
    s32 step;
    u32 rows, dataRows;
    if (s->flags & kDrawFlipY) {
        p = s->bits + (bottom + y - 1) * pitch + skipLeft + x;
        step = -pitch;
        dataRows = static_cast<u8>(h - bottom);
        rows = static_cast<u8>(static_cast<u8>(h - skipTop) - dataRows);
    } else {
        p = s->bits + (skipTop + y) * pitch + skipLeft + x;
        step = pitch;
        dataRows = static_cast<u32>(skipTop);
        rows = static_cast<u8>(bottom - skipTop);
    }
    const s32 cols = static_cast<u8>(right - skipLeft);
    const s8 *data = reinterpret_cast<const s8 *>(sprite->data);
    for (; dataRows; --dataRows) data = skipRow(data);
    // Mirrored, the row starts at its right end and the skip counts from the sprite's right.
    s32 dir = 1;
    u32 skip = static_cast<u32>(skipLeft);
    if (s->flags & kDrawFlipX) {
        dir = -1;
        skip = static_cast<u8>(w - right);
        p += cols - 1;
    }
    const bool blended = (s->flags & (kDrawGhost | kDrawGhostReverse)) != 0;
    auto draw = [&](u32 n) {
        for (; n; --n, p += dir) {
            if (!blended) *p = c.index;
            else if (s->flags & kDrawGhost) *p = s->ghost[c.index * 256u + *p];
            else *p = s->ghost[*p * 256u + c.index];
        }
    };
    u8 *next = p + step;
    for (;;) {
        s32 left = cols;
        // Columns clipped off at the start: a run straddling the edge is drawn from the edge.
        for (u32 rem = skip; rem;) {
            const s8 n = *data;
            if (n > 0) {
                if (static_cast<u32>(n) > rem) {
                    u32 count = static_cast<u8>(n - static_cast<u8>(rem));
                    if (count > static_cast<u32>(cols)) count = cols;
                    draw(count);
                    data += n + 1;
                    left = cols - static_cast<s32>(count);
                    break;
                }
                rem -= n;
                data += n + 1;
            } else if (n == 0) {
                left = 0;
                break;
            } else {
                if (n < -static_cast<s32>(rem)) {
                    const u8 sum = static_cast<u8>(rem + static_cast<u32>(n));
                    u32 gap = static_cast<u8>(-sum);
                    if (gap > static_cast<u32>(cols)) gap = cols;
                    ++data;
                    p += dir * static_cast<s32>(gap);
                    left = cols - static_cast<s32>(gap);
                    break;
                }
                rem += static_cast<u32>(n);
                ++data;
            }
        }
        // The visible columns; a run may overshoot them, ending the row.
        for (;;) {
            if (left <= 0) {
                if (--rows == 0) return;
                data = skipRow(data);
                break;
            }
            const s8 n = *data;
            if (n > 0) {
                draw(n < left ? static_cast<u32>(n) : static_cast<u8>(left));
                left -= n;
                data += n + 1;
            } else if (n < 0) {
                left += n;
                p -= dir * n;
                ++data;
            } else {
                if (--rows == 0) return;
                ++data;
                break;
            }
        }
        p = next;
        next += step;
    }
}

}  // extern "C"
