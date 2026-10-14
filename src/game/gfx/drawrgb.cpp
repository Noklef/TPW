// The drawing primitives: see draw.h. The 16-bit (0x703518) and 32-bit (0x703598) tables, one
// template each with the pixel type P.
//
// A colour is packed into a 16-bit surface's format through the context's PixelShifts, and
// written as it is (0xAARRGGBB) at 32 bits. The ghost flags do not blend at these depths: they
// only stop the remap, which (flag 0x20) otherwise takes the colour from the palette entry of the
// remapped index.
#include "draw.h"

#include "drawimpl.h"

using namespace draw_impl;

namespace {

template <typename P>
P pack(const DrawContext *s, u32 rgb);

template <>
u16 pack<u16>(const DrawContext *s, u32 rgb) {
    auto channel = [rgb](const PixelChannel &c) { return ((rgb >> (c.up & 31)) << (c.down & 31)) & c.mask; };
    const PixelShifts &p = s->shifts;
    return static_cast<u16>(channel(p.red) | channel(p.green) | channel(p.blue) | channel(p.alpha));
}

template <>
u32 pack<u32>(const DrawContext *, u32 rgb) {
    return rgb;
}

// A palette entry, opaque.
template <typename P>
P paletteColour(const DrawContext *s, u8 index) {
    const u8 *e = g_env->palette + index * 4u;
    return pack<P>(s, 0xff000000u | e[0] << 16 | e[1] << 8 | e[2]);
}

// The colour a primitive draws: remapped through the palette only without the ghost flags.
template <typename P>
P colour(const DrawContext *s, DrawColour c) {
    if (!(s->flags & (kDrawGhost | kDrawGhostReverse)) && (s->flags & kDrawRemap))
        return paletteColour<P>(s, s->remap[c.index]);
    return pack<P>(s, c.rgb);
}

template <typename P>
P *pixel(const DrawContext *s, s32 x, s32 y) {
    const s32 offset = s->surface->pitch * y + x * static_cast<s32>(sizeof(P));
    return reinterpret_cast<P *>(s->bits + static_cast<u32>(offset));
}

template <typename P>
P *nextRow(P *row, s32 pitch) {
    return reinterpret_cast<P *>(reinterpret_cast<u8 *>(row) + pitch);
}

template <typename P>
void fill(P *p, s32 count, P value) {
    for (; count > 0; --count) *p++ = value;
}

// A run-length sprite at (x, y), its literal runs drawn by colour(the run's byte). The skip of
// the clipped columns may stop with the data inside a literal; the byte there is then taken
// for a row end if it is 0 (only mirrored: zeroEnds false), dropping the rest of the row.
template <typename P, typename Colour>
void rle(s32 x, s32 y, const RleSprite *sprite, bool zeroEnds, Colour colour) {
    const DrawContext *s = g_env->state;
    const Rect &clip = s->clip;
    const s32 w = sprite->width, h = sprite->height;
    if (!w || !h) return;
    const bool flipX = (s->flags & kDrawFlipX) != 0, flipY = (s->flags & kDrawFlipY) != 0;
    // The visible size, and the data columns and rows clipped off before it (mirrored, those
    // are at the right or bottom).
    s32 cols = w, rows = h, skipX = 0, skipY = 0;
    if (clip.left > x) {
        if (!flipX) skipX = clip.left - x;
        cols += x - clip.left;
        x = clip.left;
        if (cols <= 0) return;
    }
    if (clip.right < x + cols) {
        if (flipX) skipX = x + cols - clip.right;
        cols = clip.right - x;
        if (cols <= 0) return;
    }
    if (clip.top > y) {
        if (!flipY) skipY = clip.top - y;
        rows += y - clip.top;
        y = clip.top;
        if (rows <= 0) return;
    }
    if (clip.bottom < y + rows) {
        if (flipY) skipY = y + rows - clip.bottom;
        rows = clip.bottom - y;
        if (rows <= 0) return;
    }
    if (flipX) x += cols - 1;
    s32 step = s->surface->pitch;
    if (flipY) {
        y += rows - 1;
        step = -step;
    }
    const s8 *data = reinterpret_cast<const s8 *>(sprite->data);
    for (; skipY > 0; --skipY) data = skipRow(data);
    const s32 dir = flipX ? -static_cast<s32>(sizeof(P)) : static_cast<s32>(sizeof(P));
    s32 off;
    auto draw = [&](s32 n) {
        for (; n > 0; --n, off += dir)
            *reinterpret_cast<P *>(s->bits + static_cast<u32>(off)) = colour(static_cast<u8>(*data++));
    };
    for (s32 row = s->surface->pitch * y + x * static_cast<s32>(sizeof(P)); rows > 0; --rows, row += step) {
        off = row;
        if (cols == w) {
            // Not clipped across: every run drawn.
            for (s8 n; (n = *data) != 0;) {
                ++data;
                if (n < 0) off -= n * dir;
                else draw(n);
            }
            ++data;
            continue;
        }
        // The clipped columns; a run straddling the edge leaves a gap or pixels still to draw.
        s32 rem = skipX, gap = 0, pending = 0;
        while (rem > 0) {
            const s8 n = *data;
            if (!n) break;
            if (n < 0) {
                if (-n >= rem) {
                    gap = -n - rem;
                    ++data;
                    break;
                }
                rem += n;
                ++data;
            } else {
                if (n >= rem) {
                    pending = n - rem;
                    data += rem + 1;
                    break;
                }
                rem -= n;
                data += n + 1;
            }
        }
        if ((zeroEnds || flipX) && !*data) gap = pending = 0;
        s32 left = cols;
        if (gap) {
            const s32 n = gap < left ? gap : left;
            off += n * dir;
            left -= n;
        } else if (pending) {
            const s32 n = pending < left ? pending : left;
            draw(n);
            left -= n;
            data += pending - n;
        }
        // The visible columns; a run may overshoot them, ending the row.
        while (left > 0) {
            const s8 n = *data;
            if (!n) break;
            ++data;
            if (n < 0) {
                const s32 g = -n < left ? -n : left;
                off += g * dir;
                left -= g;
            } else {
                const s32 k = n < left ? n : left;
                draw(k);
                left -= k;
                data += n - k;
            }
        }
        data = skipRow(data);
    }
}

template <typename P>
void plot(s32 x, s32 y, DrawColour c) {
    const DrawContext *s = g_env->state;
    if (inClip(s, x, y)) *pixel<P>(s, x, y) = colour<P>(s, c);
}

template <typename P>
void boxFill(const Rect *r, DrawColour c) {
    const DrawContext *s = g_env->state;
    Rect box;
    Rect_Clip(r, nullptr, &box, &s->clip);
    const s32 w = Rect_Width(&box), h = Rect_Height(&box);
    const P value = colour<P>(s, c);
    P *row = pixel<P>(s, box.left, box.top);
    for (s32 n = h; n > 0; --n, row = nextRow(row, s->surface->pitch)) fill(row, w, value);
}

template <typename P>
void outline(s32 x, s32 y, s32 r, DrawColour c) {
    const DrawContext *s = g_env->state;
    const P value = colour<P>(s, c);
    circleOutline(x, y, r, [s, value](s32 px, s32 py) {
        if (inClip(s, px, py)) *pixel<P>(s, px, py) = value;
    });
}

template <typename P>
void disc(DrawDriver *d, s32 x, s32 y, s32 r, DrawColour c) {
    const DrawContext *s = g_env->state;
    const P value = colour<P>(s, c);
    circleFill(d, x, y, r, c, [s, value](s32 px, s32 py) {
        if (inClip(s, px, py)) *pixel<P>(s, px, py) = value;
    });
}

template <typename P>
void segment(DrawDriver *d, s32 x1, s32 y1, s32 x2, s32 y2, DrawColour c) {
    const P value = colour<P>(g_env->state, c);
    line<P>(d, x1, y1, x2, y2, c, [value](u8 *p) { *reinterpret_cast<P *>(p) = value; });
}

template <typename P>
void hline(s32 x, s32 y, s32 length, DrawColour c) {
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
    fill(pixel<P>(s, x, y), length, colour<P>(s, c));
}

template <typename P>
void vline(s32 x, s32 y, s32 length, DrawColour c) {
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
    const P value = colour<P>(s, c);
    for (P *p = pixel<P>(s, x, y); length > 0; --length, p = nextRow(p, s->surface->pitch)) *p = value;
}

// The clip rectangle in the rgb as it is; flags ignored.
template <typename P>
void clear(DrawColour c) {
    const DrawContext *s = g_env->state;
    const s32 w = Rect_Width(&s->clip), h = Rect_Height(&s->clip);
    P *row = pixel<P>(s, s->clip.left, s->clip.top);
    for (s32 n = h; n > 0; --n, row = nextRow(row, s->surface->pitch)) fill(row, w, pack<P>(s, c.rgb));
}

// At 8 bits slot 14 hands the sprite to the cseg blitter; here it is drawn in place, each byte a
// palette index (no remap, no ghosting).
template <typename P>
void sprite(s32 x, s32 y, const RleSprite *sp) {
    const DrawContext *s = g_env->state;
    rle<P>(x, y, sp, false, [s](u8 index) { return paletteColour<P>(s, index); });
}

// Slot 15: the sprite's shape in one colour, the rgb packed as it is (no remap, no ghosting).
template <typename P>
void shape(s32 x, s32 y, const RleSprite *sp, DrawColour c) {
    const P value = pack<P>(g_env->state, c.rgb);
    rle<P>(x, y, sp, true, [value](u8) { return value; });
}

}  // namespace

extern "C" {

void __fastcall Draw16_Pixel(DrawDriver *, void *, s32 x, s32 y, DrawColour c) { plot<u16>(x, y, c); }
void __fastcall Draw16_BoxFill(DrawDriver *, void *, const Rect *r, DrawColour c) { boxFill<u16>(r, c); }
void __fastcall Draw16_CircleOutline(DrawDriver *, void *, s32 x, s32 y, s32 r, DrawColour c) {
    outline<u16>(x, y, r, c);
}
void __fastcall Draw16_CircleFill(DrawDriver *d, void *, s32 x, s32 y, s32 r, DrawColour c) {
    disc<u16>(d, x, y, r, c);
}
void __fastcall Draw16_Line(DrawDriver *d, void *, s32 x1, s32 y1, s32 x2, s32 y2, DrawColour c) {
    segment<u16>(d, x1, y1, x2, y2, c);
}
void __fastcall Draw16_HLine(DrawDriver *, void *, s32 x, s32 y, s32 length, DrawColour c) {
    hline<u16>(x, y, length, c);
}
void __fastcall Draw16_VLine(DrawDriver *, void *, s32 x, s32 y, s32 length, DrawColour c) {
    vline<u16>(x, y, length, c);
}
void __fastcall Draw16_Clear(DrawDriver *, void *, DrawColour c) { clear<u16>(c); }
void __fastcall Draw16_Text(DrawDriver *, void *, s32 x, s32 y, const char *str, DrawColour c) {
    text<u16>(x, y, reinterpret_cast<const signed char *>(str), pack<u16>(g_env->state, c.rgb));
}
void __fastcall Draw16_TextW(DrawDriver *, void *, s32 x, s32 y, const u16 *str, DrawColour c) {
    text<u16>(x, y, str, pack<u16>(g_env->state, c.rgb));
}
void __fastcall Draw16_Sprite(DrawDriver *, void *, s32 x, s32 y, const RleSprite *sp) { sprite<u16>(x, y, sp); }
void __fastcall Draw16_SpriteShape(DrawDriver *, void *, s32 x, s32 y, const RleSprite *sp, DrawColour c) {
    shape<u16>(x, y, sp, c);
}

void __fastcall Draw32_Pixel(DrawDriver *, void *, s32 x, s32 y, DrawColour c) { plot<u32>(x, y, c); }
void __fastcall Draw32_BoxFill(DrawDriver *, void *, const Rect *r, DrawColour c) { boxFill<u32>(r, c); }
void __fastcall Draw32_CircleOutline(DrawDriver *, void *, s32 x, s32 y, s32 r, DrawColour c) {
    outline<u32>(x, y, r, c);
}
void __fastcall Draw32_CircleFill(DrawDriver *d, void *, s32 x, s32 y, s32 r, DrawColour c) {
    disc<u32>(d, x, y, r, c);
}
void __fastcall Draw32_Line(DrawDriver *d, void *, s32 x1, s32 y1, s32 x2, s32 y2, DrawColour c) {
    segment<u32>(d, x1, y1, x2, y2, c);
}
void __fastcall Draw32_HLine(DrawDriver *, void *, s32 x, s32 y, s32 length, DrawColour c) {
    hline<u32>(x, y, length, c);
}
void __fastcall Draw32_VLine(DrawDriver *, void *, s32 x, s32 y, s32 length, DrawColour c) {
    vline<u32>(x, y, length, c);
}
void __fastcall Draw32_Clear(DrawDriver *, void *, DrawColour c) { clear<u32>(c); }
void __fastcall Draw32_Text(DrawDriver *, void *, s32 x, s32 y, const char *str, DrawColour c) {
    text<u32>(x, y, reinterpret_cast<const signed char *>(str), pack<u32>(g_env->state, c.rgb));
}
void __fastcall Draw32_TextW(DrawDriver *, void *, s32 x, s32 y, const u16 *str, DrawColour c) {
    text<u32>(x, y, str, pack<u32>(g_env->state, c.rgb));
}
void __fastcall Draw32_Sprite(DrawDriver *, void *, s32 x, s32 y, const RleSprite *sp) { sprite<u32>(x, y, sp); }
void __fastcall Draw32_SpriteShape(DrawDriver *, void *, s32 x, s32 y, const RleSprite *sp, DrawColour c) {
    shape<u32>(x, y, sp, c);
}

}  // extern "C"
