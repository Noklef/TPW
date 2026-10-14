// The drawing primitives: see draw.h. The shared slots and the 8-bit table.
#include "draw.h"

#include "../core/vcall.h"
#include "drawimpl.h"

#include <cstdint>

using namespace draw_impl;

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
    at<const u8 *>(0x00fa71f8),
};

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

}  // namespace

namespace draw_impl {
const DrawEnv *g_env = &kGameEnv;
}

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
    const DrawContext *s = g_env->state;
    circleOutline(x, y, r, [s, c](s32 px, s32 py) { plot(s, px, py, c.index); });
}

void __fastcall Draw8_CircleFill(DrawDriver *d, void *, s32 x, s32 y, s32 r, DrawColour c) {
    const DrawContext *s = g_env->state;
    circleFill(d, x, y, r, c, [s, c](s32 px, s32 py) { plot(s, px, py, c.index); });
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
    const DrawContext *s = g_env->state;
    const bool blended = (s->flags & (kDrawGhost | kDrawGhostReverse)) != 0;
    const u8 index = blended ? c.index : plain(s, c.index);
    line<u8>(d, x1, y1, x2, y2, c, [s, index](u8 *p) {
        if (s->flags & kDrawGhost) *p = s->ghost[index * 256u + *p];
        else if (s->flags & kDrawGhostReverse) *p = s->ghost[*p * 256u + index];
        else *p = index;
    });
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
    text<u8>(x, y, reinterpret_cast<const signed char *>(str), c.index);
}

void __fastcall Draw8_TextW(DrawDriver *, void *, s32 x, s32 y, const u16 *str, DrawColour c) {
    text<u8>(x, y, str, c.index);
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
