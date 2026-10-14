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

// A palette entry, opaque.
u16 paletteColour(const DrawContext *s, u8 index) {
    const u8 *e = g_env->palette + index * 4u;
    return pack(s, 0xff000000u | e[0] << 16 | e[1] << 8 | e[2]);
}

// The colour a primitive draws: remapped through the palette only without the ghost flags.
u16 colour(const DrawContext *s, DrawColour c) {
    if (!(s->flags & (kDrawGhost | kDrawGhostReverse)) && (s->flags & kDrawRemap))
        return paletteColour(s, s->remap[c.index]);
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

void __fastcall Draw16_CircleOutline(DrawDriver *, void *, s32 x, s32 y, s32 r, DrawColour c) {
    const DrawContext *s = g_env->state;
    const u16 value = colour(s, c);
    circleOutline(x, y, r, [s, value](s32 px, s32 py) {
        if (inClip(s, px, py)) *pixel(s, px, py) = value;
    });
}

void __fastcall Draw16_CircleFill(DrawDriver *d, void *, s32 x, s32 y, s32 r, DrawColour c) {
    const DrawContext *s = g_env->state;
    const u16 value = colour(s, c);
    circleFill(d, x, y, r, c, [s, value](s32 px, s32 py) {
        if (inClip(s, px, py)) *pixel(s, px, py) = value;
    });
}

void __fastcall Draw16_Line(DrawDriver *d, void *, s32 x1, s32 y1, s32 x2, s32 y2, DrawColour c) {
    const u16 value = colour(g_env->state, c);
    line<u16>(d, x1, y1, x2, y2, c, [value](u8 *p) { *reinterpret_cast<u16 *>(p) = value; });
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

// At 8 bits slot 14 hands the sprite to the cseg blitter; here it is drawn in place, each byte a
// palette index (no remap, no ghosting).
void __fastcall Draw16_Sprite(DrawDriver *, void *, s32 x, s32 y, const RleSprite *sprite) {
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
    const s32 dir = flipX ? -2 : 2;
    s32 off;
    auto draw = [&](s32 n) {
        for (; n > 0; --n, off += dir) *reinterpret_cast<u16 *>(s->bits + static_cast<u32>(off)) = paletteColour(s, *data++);
    };
    for (s32 row = s->surface->pitch * y + x * 2; rows > 0; --rows, row += step) {
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
        // Mirrored, the next byte is taken for a row end even when it is the next pixel: a
        // straddling run whose first visible index is 0 ends the row.
        if (flipX && !*data) gap = pending = 0;
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

}  // extern "C"
