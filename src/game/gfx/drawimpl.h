// Shared by the drawing primitives' depths (draw8.cpp, draw16.cpp): not part of the interface.
#pragma once

#include "draw.h"

namespace draw_impl {

extern const DrawEnv *g_env;

inline bool inClip(const DrawContext *s, s32 x, s32 y) {
    return s->clip.left <= x && x < s->clip.right && s->clip.top <= y && y < s->clip.bottom;
}

// a * b / c as imul, cdq, idiv: the product wraps to 32 bits.
inline s32 mulDiv(s32 a, s32 b, s32 c) {
    return static_cast<s32>(static_cast<u32>(a) * static_cast<u32>(b)) / c;
}

// Clip one axis of a line to [lo, hi): u is the axis clipped, v the other. False: the line is
// entirely outside. Both ends are moved by interpolation; the far end of the other axis is not
// checked again.
inline bool clipAxis(s32 &u1, s32 &v1, s32 &u2, s32 &v2, s32 lo, s32 hi) {
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
inline const s8 *skipRow(const s8 *data) {
    for (s8 n; (n = *data) != 0;) data += n > 0 ? n + 1 : 1;
    return data + 1;
}

// The 5x6 debug font: glyph rows 0x40..0x04, the advance in byte 6. Pixels of type P (one per
// sizeof(P) bytes) are set to value; no flags apply.
template <typename P, typename C>
void text(s32 x, s32 y, const C *str, P value) {
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
                u8 *row = s->bits + static_cast<u32>(pitch * y) + cx * sizeof(P);
                for (s32 r = 0; r < 6; ++r, row += pitch) {
                    P *p = reinterpret_cast<P *>(row);
                    const u8 bits = g[r];
                    if (bits & 0x40) p[0] = value;
                    if (bits & 0x20) p[1] = value;
                    if (bits & 0x10) p[2] = value;
                    if (bits & 0x08) p[3] = value;
                    if (bits & 0x04) p[4] = value;
                }
            }
        }
        cx += advance;
    }
}

}  // namespace draw_impl
