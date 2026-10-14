// The canvas primitives' pixel, spans and outline: see canvasdraw.h.
#include "canvasdraw.h"

#include <cstdint>
#include <cstring>
#include <utility>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const CanvasDrawEnv kGameEnv = {
    at<u32(__fastcall *)(Color *, void *, const PixelFormat *)>(0x005fd050),
    at<void(__fastcall *)(Rect *)>(0x0062f940),
    at<u32(__fastcall *)(const Rect *, void *, const Rect *)>(0x0062f890),
    at<Rect *(__fastcall *)(const Rect *, void *, Rect *, const Rect *)>(0x0062f810),
    at<const u8 *>(0x007796e0),
    at<const s32 *>(0x00fa76cc),
};
const CanvasDrawEnv *g_env = &kGameEnv;

using SpanFn = void(__fastcall *)(CanvasDriver *, void *, const SurfaceInfo *, const Rect *, u32, s32, s32, s32,
                                  Color *);
using LineFn = void(__fastcall *)(CanvasDriver *, void *, const SurfaceInfo *, const Rect *, u32, s32, s32, s32, s32,
                                  Color *);

s32 add(s32 a, s32 b) { return static_cast<s32>(static_cast<u32>(a) + static_cast<u32>(b)); }
s32 sub(s32 a, s32 b) { return static_cast<s32>(static_cast<u32>(a) - static_cast<u32>(b)); }

// The pixel at (x, y) of a Bytes-wide surface.
template <u32 Bytes>
u8 *pixelAt(const SurfaceInfo *s, s32 x, s32 y) {
    const u32 a = static_cast<u32>(reinterpret_cast<std::uintptr_t>(s->bits)) +
                  static_cast<u32>(s->pitch) * static_cast<u32>(y) + static_cast<u32>(x) * Bytes;
    return reinterpret_cast<u8 *>(static_cast<std::uintptr_t>(a));
}
// The destination as the blend reads it: 24 bits as a whole dword (sic).
template <u32 Bytes>
u32 load(const u8 *p) {
    u32 v = 0;
    std::memcpy(&v, p, Bytes == 3 ? 4 : Bytes);
    return v;
}
template <u32 Bytes>
void store(u8 *p, u32 v) {
    std::memcpy(p, &v, Bytes);
}
// The colour's pixel as the blend uses it: 24 and 32 bits whole.
template <u32 Bytes>
u32 source(u32 pixel) {
    return Bytes == 1 ? pixel & 0xff : Bytes == 2 ? pixel & 0xffff : pixel;
}
// The colour's alpha (read after the conversion), inverted by flag 0x10.
u32 alphaOf(const Color *c, u32 flags) {
    const u8 a = reinterpret_cast<const u8 *>(c)[3];
    return flags & 0x10 ? static_cast<u8>(0xff - a) : a;
}
u32 blendChannel(u32 mask, u32 srcMasked, u32 dst, u32 alpha) {
    return (((srcMasked - (mask & dst)) * alpha >> 8) + (mask & dst)) & mask;
}
// The spans' blend.
u32 blend(const SurfaceInfo *s, const u32 srcMasked[3], u32 dst, u32 alpha) {
    return blendChannel(s->redMask, srcMasked[0], dst, alpha) | blendChannel(s->greenMask, srcMasked[1], dst, alpha) |
           blendChannel(s->blueMask, srcMasked[2], dst, alpha) | (s->alphaMask & dst);
}
// The pixel's: (mask - (mask & dst)) & src in place of (mask & src) - (mask & dst).
u32 pixelChannel(u32 mask, u32 src, u32 dst, u32 alpha) {
    return ((((mask - (mask & dst)) & src) * alpha >> 8) + (mask & dst)) & mask;
}

template <u32 Bytes>
void drawPixel(const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x, s32 y, Color *colour) {
    const u32 pixel = g_env->toPixel(colour, nullptr, reinterpret_cast<const PixelFormat *>(s));
    if (x < clip->left || x >= clip->right || y < clip->top || y >= clip->bottom) return;
    u8 *p = pixelAt<Bytes>(s, x, y);
    if (!(flags & 0x18)) {
        store<Bytes>(p, pixel);
        return;
    }
    const u32 alpha = alphaOf(colour, flags), src = source<Bytes>(pixel), dst = load<Bytes>(p);
    store<Bytes>(p, pixelChannel(s->redMask, src, dst, alpha) | pixelChannel(s->greenMask, src, dst, alpha) |
                        pixelChannel(s->blueMask, src, dst, alpha) | (s->alphaMask & dst));
}

// A span of len pixels from p, step bytes apart; Stuck blends the first pixel throughout (sic).
template <u32 Bytes, bool Stuck>
void drawSpan(const SurfaceInfo *s, u8 *p, s32 step, u32 len, u32 flags, u32 pixel, const Color *colour) {
    if (!(flags & 0x18)) {
        for (; len; --len, p += step) store<Bytes>(p, pixel);
        return;
    }
    const u32 src = source<Bytes>(pixel);
    const u32 srcMasked[3] = {s->redMask & src, s->greenMask & src, s->blueMask & src};
    const u32 alpha = alphaOf(colour, flags);
    for (; len; --len) {
        store<Bytes>(p, blend(s, srcMasked, load<Bytes>(p), alpha));
        if (!Stuck) p += step;
    }
}

template <u32 Bytes>
void drawHSpan(const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x, s32 y, s32 len, Color *colour) {
    const u32 pixel = g_env->toPixel(colour, nullptr, reinterpret_cast<const PixelFormat *>(s));
    if (len < 0) {
        x = add(x, len);
        len = sub(0, len);
    }
    if (y < clip->top || y >= clip->bottom || add(x, len) < clip->left || x >= clip->right) return;
    if (x < clip->left) {
        len = add(len, sub(x, clip->left));
        x = clip->left;
    }
    if (add(x, len) > clip->right) len = sub(clip->right, x);
    drawSpan<Bytes, Bytes == 3>(s, pixelAt<Bytes>(s, x, y), Bytes, static_cast<u32>(len), flags, pixel, colour);
}

template <u32 Bytes>
void drawVSpan(const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x, s32 y, s32 len, Color *colour) {
    const u32 pixel = g_env->toPixel(colour, nullptr, reinterpret_cast<const PixelFormat *>(s));
    if (len < 0) {
        y = add(y, len);
        len = sub(0, len);
    }
    if (add(y, len) < clip->top || y >= clip->bottom || x < clip->left || x >= clip->right) return;
    if (y < clip->top) {
        len = add(len, sub(y, clip->top));
        y = clip->top;
    }
    if (add(y, len) > clip->bottom) len = sub(clip->bottom, y);
    drawSpan<Bytes, false>(s, pixelAt<Bytes>(s, x, y), s->pitch, static_cast<u32>(len), flags, pixel, colour);
}

s32 mul(s32 a, s32 b) { return static_cast<s32>(static_cast<u32>(a) * static_cast<u32>(b)); }

// The line clipped as the original does: x first, then y, the far end moved along the line to
// the edge (right and bottom less one), nothing re-checked. False when it misses.
bool clipLine(const Rect *c, s32 &x0, s32 &y0, s32 &x1, s32 &y1) {
    if (x0 < c->left) {
        if (x1 < c->left) return false;
        y0 = sub(y0, mul(sub(x0, c->left), sub(y0, y1)) / sub(x0, x1));
        x0 = c->left;
        if (x1 >= c->right) {
            y1 = sub(y1, mul(add(sub(x1, c->right), 1), sub(y1, y0)) / sub(x1, x0));
            x1 = sub(c->right, 1);
        }
    } else if (x0 >= c->right) {
        if (x1 >= c->right) return false;
        y0 = sub(y0, mul(add(sub(x0, c->right), 1), sub(y0, y1)) / sub(x0, x1));
        x0 = sub(c->right, 1);
        if (x1 < c->left) {
            y1 = sub(y1, mul(sub(x1, c->left), sub(y1, y0)) / sub(x1, x0));
            x1 = c->left;
        }
    } else if (x1 < c->left) {
        y1 = sub(y1, mul(sub(x1, c->left), sub(y1, y0)) / sub(x1, x0));
        x1 = c->left;
    } else if (x1 >= c->right) {
        y1 = sub(y1, mul(add(sub(x1, c->right), 1), sub(y1, y0)) / sub(x1, x0));
        x1 = sub(c->right, 1);
    }
    if (y0 < c->top) {
        if (y1 < c->top) return false;
        x0 = sub(x0, mul(sub(y0, c->top), sub(x0, x1)) / sub(y0, y1));
        y0 = c->top;
        if (y1 >= c->bottom) {
            x1 = sub(x1, mul(add(sub(y1, c->bottom), 1), sub(x1, x0)) / sub(y1, y0));
            y1 = sub(c->bottom, 1);
        }
    } else if (y0 >= c->bottom) {
        if (y1 >= c->bottom) return false;
        x0 = sub(x0, mul(add(sub(y0, c->bottom), 1), sub(x0, x1)) / sub(y0, y1));
        y0 = sub(c->bottom, 1);
        if (y1 < c->top) {
            x1 = sub(x1, mul(sub(y1, c->top), sub(x1, x0)) / sub(y1, y0));
            y1 = c->top;
        }
    } else if (y1 < c->top) {
        x1 = sub(x1, mul(sub(y1, c->top), sub(x1, x0)) / sub(y1, y0));
        y1 = c->top;
    } else if (y1 >= c->bottom) {
        x1 = sub(x1, mul(add(sub(y1, c->bottom), 1), sub(x1, x0)) / sub(y1, y0));
        y1 = sub(c->bottom, 1);
    }
    return true;
}

template <u32 Bytes>
void drawLine(CanvasDriver *d, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x0, s32 y0, s32 x1, s32 y1,
              Color *colour) {
    if (x0 == x1) {
        reinterpret_cast<SpanFn>(d->table[3])(d, nullptr, s, clip, flags, x0, y0, sub(y1, y0), colour);
        return;
    }
    if (y0 == y1) {
        reinterpret_cast<SpanFn>(d->table[2])(d, nullptr, s, clip, flags, x0, y0, sub(x1, x0), colour);
        return;
    }
    if (!clipLine(clip, x0, y0, x1, y1)) return;
    const u32 pixel = g_env->toPixel(colour, nullptr, reinterpret_cast<const PixelFormat *>(s));
    const s32 dx = sub(x1, x0), dy = sub(y1, y0);
    const s32 ax = mul(dx < 0 ? sub(0, dx) : dx, 2), ay = mul(dy < 0 ? sub(0, dy) : dy, 2);
    const s32 sx = dx > 0 ? 1 : -1, sy = dy > 0 ? 1 : -1;
    const s32 stepX = sx * static_cast<s32>(Bytes), stepY = mul(sy, s->pitch);
    const bool blended = (flags & 0x18) != 0;
    const u32 src = source<Bytes>(pixel);
    u32 srcMasked[3] = {}, alpha = 0;
    if (blended) {
        alpha = alphaOf(colour, flags);
        srcMasked[0] = s->redMask & src;
        srcMasked[1] = s->greenMask & src;
        srcMasked[2] = s->blueMask & src;
    }
    auto plot = [&](u8 *at) {
        store<Bytes>(at, blended ? blend(s, srcMasked, load<Bytes>(at), alpha) : pixel);
    };
    u8 *p = pixelAt<Bytes>(s, x0, y0);
    plot(p);
    if (ax > ay) {
        for (s32 e = sub(ay, ax >> 1); x0 != x1;) {
            if (e >= 0) {
                p += stepY;
                e = sub(e, ax);
            }
            x0 = add(x0, sx);
            p += stepX;
            e = add(e, ay);
            plot(p);
        }
    } else {
        for (s32 e = sub(ax, ay >> 1); y0 != y1;) {
            if (e >= 0) {
                p += stepX;
                e = sub(e, ay);
            }
            y0 = add(y0, sy);
            p += stepY;
            e = add(e, ax);
            plot(p);
        }
    }
}

bool inside(const Rect *clip, s32 x, s32 y) {
    return x >= clip->left && x < clip->right && y >= clip->top && y < clip->bottom;
}
// A 16.16 edge step: (dx << 16) / dy.
s32 slope(s32 dx, s32 dy) { return static_cast<s32>(static_cast<u32>(dx) << 16) / dy; }

template <u32 Bytes>
void drawTriangle(CanvasDriver *d, const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, Color *colour) {
    if (flags & 4) {
        u8 *const p[3] = {pixelAt<Bytes>(s, x0, y0), pixelAt<Bytes>(s, x1, y1), pixelAt<Bytes>(s, x2, y2)};
        u32 saved[3] = {};
        bool kept[3] = {};
        auto keep = [&](int i, s32 x, s32 y) {
            if (!inside(clip, x, y)) return;
            saved[i] = load<Bytes>(p[i]);
            kept[i] = true;
        };
        reinterpret_cast<LineFn>(d->table[4])(d, nullptr, s, clip, flags, x0, y0, x1, y1, colour);
        keep(0, x0, y0);
        keep(1, x1, y1);
        reinterpret_cast<LineFn>(d->table[4])(d, nullptr, s, clip, flags, x1, y1, x2, y2, colour);
        keep(2, x2, y2);
        reinterpret_cast<LineFn>(d->table[4])(d, nullptr, s, clip, flags, x2, y2, x0, y0, colour);
        for (int i = 0; i < 3; ++i)
            if (kept[i]) store<Bytes>(p[i], saved[i]);
        return;
    }
    if (y0 > y1) std::swap(x0, x1), std::swap(y0, y1);
    if (y1 > y2) std::swap(x1, x2), std::swap(y1, y2);
    if (y0 > y1) std::swap(x0, x1), std::swap(y0, y1);
    // Edge a steps by da and edge b by db; each row spans from the left one to the right one.
    s32 a, b, da, db, y = y0;
    auto rows = [&](s32 end, bool aLeft) {
        for (; y < end; y = add(y, 1), a = add(a, da), b = add(b, db)) {
            const s32 l = (aLeft ? a : b) >> 16, r = (aLeft ? b : a) >> 16;
            reinterpret_cast<SpanFn>(d->table[2])(d, nullptr, s, clip, flags, l, y, sub(r, l), colour);
        }
    };
    if (y0 == y1) {
        if (y1 == y2) return;
        if (x0 < x1) {
            a = static_cast<s32>(static_cast<u32>(x0) << 16), da = slope(sub(x2, x0), sub(y2, y0));
            b = static_cast<s32>(static_cast<u32>(x1) << 16), db = slope(sub(x2, x1), sub(y2, y1));
        } else {
            a = static_cast<s32>(static_cast<u32>(x1) << 16), da = slope(sub(x2, x1), sub(y2, y1));
            b = static_cast<s32>(static_cast<u32>(x0) << 16), db = slope(sub(x2, x0), sub(y2, y0));
        }
        rows(y2, true);
        return;
    }
    a = b = static_cast<s32>(static_cast<u32>(x0) << 16);
    da = slope(sub(x2, x0), sub(y2, y0));
    db = slope(sub(x1, x0), sub(y1, y0));
    const bool aLeft = da < db;
    rows(y1, aLeft);
    if (y1 == y2) return;
    b = static_cast<s32>(static_cast<u32>(x1) << 16);
    db = slope(sub(x2, x1), sub(y2, y1));
    rows(y2, aLeft);
}

// Past the rest of a row's codes and its terminator.
const u8 *skipRow(const u8 *p) {
    for (s8 c; (c = static_cast<s8>(*p)) != 0;) p += c > 0 ? c + 1 : 1;
    return p + 1;
}
template <u32 Bytes>
u8 *fillRun(u8 *q, s32 n, s32 step, u32 pixel) {
    for (; n > 0; --n) {
        store<Bytes>(q, pixel);
        q += step;
    }
    return q;
}

// A mask or sprite clipped: its first pixel (the right or bottom end when flipped), the size
// left, the columns and rows to pass and the row step. False when nothing is left.
struct MaskClip {
    s32 x, y, w, h, skipX, skipY, stride;
};
bool clipMask(const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x, s32 y, const CanvasMask *m, MaskClip *o) {
    if (!m->width || !m->height) return false;
    s32 w = m->width, h = m->height, skipX = 0, skipY = 0;
    if (clip->left > x) {
        if (!(flags & 1)) skipX = sub(clip->left, x);
        w = add(w, sub(x, clip->left));
        x = clip->left;
        if (w <= 0) return false;
    }
    if (clip->right < add(x, w)) {
        if (flags & 1) skipX = sub(add(w, x), clip->right);
        w = sub(clip->right, x);
        if (w <= 0) return false;
    }
    if (clip->top > y) {
        if (!(flags & 2)) skipY = sub(clip->top, y);
        h = add(h, sub(y, clip->top));
        y = clip->top;
        if (h <= 0) return false;
    }
    if (clip->bottom < add(y, h)) {
        if (flags & 2) skipY = sub(add(h, y), clip->bottom);
        h = sub(clip->bottom, y);
        if (h <= 0) return false;
    }
    s32 stride = s->pitch;
    if (flags & 1) x = sub(add(x, w), 1);
    if (flags & 2) {
        stride = sub(0, stride);
        y = sub(add(y, h), 1);
    }
    *o = {x, y, w, h, skipX, skipY, stride};
    return true;
}
// Passes left columns of a row's codes; a skip or run they end inside is carried over in skip or
// run (both dropped, when atZero, if the byte reached is 0).
const u8 *passColumns(const u8 *p, s32 left, s32 &skip, s32 &run, bool atZero = true) {
    skip = run = 0;
    while (left > 0 && *p) {
        const s8 c = static_cast<s8>(*p);
        if (c < 0) {
            if (-c >= left) {
                skip = sub(0, add(left, c));
                ++p;
                break;
            }
            left += c;
            ++p;
        } else {
            if (c >= left) {
                run = c - left;
                p += left + 1;
                break;
            }
            left -= c;
            p += c + 1;
        }
    }
    if (atZero && !*p) skip = run = 0;
    return p;
}

template <u32 Bytes>
void drawMask(const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x, s32 y, const CanvasMask *m, Color *colour) {
    MaskClip k;
    if (!clipMask(s, clip, flags, x, y, m, &k)) return;
    const bool flipX = (flags & 1) != 0;
    const u8 *p = m->codes;
    for (s32 i = k.skipY; i > 0; --i) p = skipRow(p);
    const u32 pixel = g_env->toPixel(colour, nullptr, reinterpret_cast<const PixelFormat *>(s));
    u8 *row = pixelAt<Bytes>(s, k.x, k.y);
    const s32 B = static_cast<s32>(Bytes);
    if (k.w == m->width) {
        // Unclipped across: the codes as they come. Flipped it still draws forwards, from the
        // right end, and a skip passes the code after it too.
        for (; k.h > 0; --k.h, row += k.stride) {
            u8 *q = row;
            for (s8 c; (c = static_cast<s8>(*p)) != 0;) {
                if (c < 0) {
                    q += sub(0, c) * B;
                    p += flipX ? 2 : 1;
                } else {
                    p += c + 1;
                    q = fillRun<Bytes>(q, c, B, pixel);
                }
            }
            ++p;
        }
        return;
    }
    // Clipped across: at most w drawn, backwards when flipped: a byte a pixel (sic) but for a
    // carried-over skip.
    const s32 step = flipX ? -1 : B;
    for (; k.h > 0; --k.h, row += k.stride) {
        s32 skip, run;
        p = passColumns(p, k.skipX, skip, run);
        u8 *q = row;
        s32 left = k.w;
        if (skip) {
            if (skip < left) {
                left -= skip;
                q += skip * (flipX ? -B : B);
            } else left = 0;
        } else if (run) {
            p += run;
            if (run < left) {
                left -= run;
                q = fillRun<Bytes>(q, run, step, pixel);
            } else {
                fillRun<Bytes>(q, left, step, pixel);
                left = 0;
            }
        }
        for (s8 c; left > 0 && (c = static_cast<s8>(*p)) != 0;) {
            p += c < 0 ? 1 : c + 1;
            if (c < 0) {
                if (-c < left) {
                    left += c;
                    q += -c * step;
                } else left = 0;
            } else if (c < left) {
                left -= c;
                q = fillRun<Bytes>(q, c, step, pixel);
            } else {
                fillRun<Bytes>(q, left, step, pixel);
                left = 0;
            }
        }
        p = skipRow(p);
    }
}

// n pixels from the table by the indices at p, from q by step bytes.
template <u32 Bytes>
u8 *copyRun(u8 *q, const u8 *&p, s32 n, s32 step, const u32 *table) {
    for (; n > 0; --n) {
        if constexpr (Bytes == 3) {  // a byte at a time (the table may be the destination)
            const u8 *e = reinterpret_cast<const u8 *>(&table[*p++]);
            for (int i = 0; i < 3; ++i) q[i] = e[i];
        } else store<Bytes>(q, table[*p++]);
        q += step;
    }
    return q;
}

template <u32 Bytes>
void drawSprite(const SurfaceInfo *s, const Rect *clip, u32 flags, const u32 *table, s32 x, s32 y,
                const CanvasMask *m) {
    MaskClip k;
    if (!clipMask(s, clip, flags, x, y, m, &k)) return;
    const u8 *p = m->codes;
    for (s32 i = k.skipY; i > 0; --i) p = skipRow(p);
    u8 *row = pixelAt<Bytes>(s, k.x, k.y);
    // The 24-bit code steps 4 bytes a pixel (sic).
    const s32 B = Bytes == 3 ? 4 : static_cast<s32>(Bytes), step = flags & 1 ? -B : B;
    const bool clipped = k.w != m->width;
    for (; k.h > 0; --k.h, row += k.stride) {
        u8 *q = row;
        s32 left = k.w;
        if (clipped) {
            s32 skip, run;
            p = passColumns(p, k.skipX, skip, run, (flags & 1) != 0);
            if (skip) {
                if (skip < left) {
                    left -= skip;
                    q += skip * step;
                } else left = 0;
            } else if (run) {
                const s32 n = run < left ? run : left;
                q = copyRun<Bytes>(q, p, n, step, table);
                p += run - n;
                left = run < left ? left - run : 0;
            }
        }
        for (s8 c; (!clipped || left > 0) && (c = static_cast<s8>(*p)) != 0;) {
            ++p;
            if (!clipped) {
                q = c < 0 ? q + -c * step : copyRun<Bytes>(q, p, c, step, table);
            } else if (c < 0) {
                if (-c < left) {
                    left += c;
                    q += -c * step;
                } else left = 0;
            } else {
                const s32 n = c < left ? c : left;
                q = copyRun<Bytes>(q, p, n, step, table);
                p += c - n;
                left = c < left ? left - c : 0;
            }
        }
        p = clipped ? skipRow(p) : p + 1;
    }
}

template <u32 Bytes>
void fillRect(const SurfaceInfo *s, const Rect *clip, u32 flags, const Rect *rect, Color *colour) {
    Rect r = *rect;
    g_env->order(&r);
    if (!g_env->overlaps(&r, nullptr, clip)) return;
    Rect o;
    g_env->clip(&r, nullptr, &o, clip);
    u8 *row = pixelAt<Bytes>(s, o.left, o.top);
    const u32 pixel = g_env->toPixel(colour, nullptr, reinterpret_cast<const PixelFormat *>(s));
    const u32 cols = static_cast<u32>(sub(o.right, o.left));
    for (u32 rows = static_cast<u32>(sub(o.bottom, o.top)); rows; --rows, row += s->pitch)
        drawSpan<Bytes, Bytes == 3>(s, row, Bytes, cols, flags, pixel, colour);
}

template <u32 Bytes>
void drawCircle(const SurfaceInfo *s, const Rect *clip, u32 flags, s32 cx, s32 cy, s32 r, Color *colour) {
    const u32 pixel = g_env->toPixel(colour, nullptr, reinterpret_cast<const PixelFormat *>(s));
    const bool blended = (flags & 0x18) != 0;
    const u32 src = source<Bytes>(pixel);
    const u32 srcMasked[3] = {s->redMask & src, s->greenMask & src, s->blueMask & src};
    const u32 alpha = alphaOf(colour, flags);
    bool checked = true;
    auto put = [&](s32 x, s32 y) {
        if (checked && !inside(clip, x, y)) return;
        u8 *p = pixelAt<Bytes>(s, x, y);
        store<Bytes>(p, blended ? blend(s, srcMasked, load<Bytes>(p), alpha) : pixel);
    };
    if (r == 0) {
        put(cx, cy);
        return;
    }
    const Rect bounds = {sub(cx, r), sub(cy, r), add(add(cx, r), 1), add(add(cy, r), 1)};
    if (!g_env->overlaps(clip, nullptr, &bounds)) return;
    if (r == 1) {
        put(sub(cx, 1), cy);
        put(cx, sub(cy, 1));
        put(add(cx, 1), cy);
        put(cx, add(cy, 1));
        put(cx, cy);
        return;
    }
    // Unchecked when the box is wholly inside: a negative radius's box is inside out, so its ends can
    // land outside the clip. (The unchecked code also orders its points by column, which never shows.)
    checked = clip->left > bounds.left || clip->top > bounds.top || clip->right < bounds.right ||
              clip->bottom < bounds.bottom;
    u32 d = 3 - 2 * static_cast<u32>(r);
    s32 x = 0, y = r;
    auto advance = [&] {
        if (static_cast<s32>(d) < 0) d += 4 * static_cast<u32>(x) + 6;
        else d += 4 * static_cast<u32>(sub(x, y)) + 10, --y;
        ++x;
    };
    auto four = [&](s32 dx, s32 dy) {  // (cx -+ dx, cy -+ dy), across then down
        put(sub(cx, dx), sub(cy, dy)), put(add(cx, dx), sub(cy, dy));
        put(sub(cx, dx), add(cy, dy)), put(add(cx, dx), add(cy, dy));
    };
    if (!blended) {
        for (; x <= y; advance()) four(x, y), four(y, x);  // a negative radius never enters
        return;
    }
    // Blended, each pixel once: the four ends, the octants between, then the four diagonals.
    put(cx, sub(cy, r)), put(cx, add(cy, r)), put(sub(cx, r), cy), put(add(cx, r), cy);
    for (advance(); x < y; advance()) four(x, y), four(y, x);
    if (x == y) four(x, x);
}

s32 clampX(const Rect *clip, s32 x) { return x < clip->left ? clip->left : x > clip->right ? clip->right : x; }

template <u32 Bytes>
void drawDisc(const SurfaceInfo *s, const Rect *clip, u32 flags, s32 cx, s32 cy, s32 r, Color *colour) {
    const u32 pixel = g_env->toPixel(colour, nullptr, reinterpret_cast<const PixelFormat *>(s));
    auto put = [&](s32 x, s32 y) {
        if (!inside(clip, x, y)) return;
        drawSpan<Bytes, false>(s, pixelAt<Bytes>(s, x, y), Bytes, 1, flags, pixel, colour);
    };
    if (r == 0) {
        put(cx, cy);
        return;
    }
    const Rect bounds = {sub(cx, r), sub(cy, r), add(add(cx, r), 1), add(add(cy, r), 1)};
    if (!g_env->overlaps(clip, nullptr, &bounds)) return;
    if (r == 1) {
        put(sub(cx, 1), cy);
        put(cx, sub(cy, 1));
        put(add(cx, 1), cy);
        put(cx, add(cy, 1));
        put(cx, cy);
        return;
    }
    // Rows from xs to xe, clamped across; the first pixel found by the surface's own depth (sic).
    const u32 depthBytes = (s->bitCount + 7) >> 3;
    auto span = [&](s32 y, s32 xs, s32 xe) {
        const s32 from = clampX(clip, xs), n = sub(clampX(clip, xe), from);
        if (y < clip->top || y >= clip->bottom) return;
        const u32 a = static_cast<u32>(reinterpret_cast<std::uintptr_t>(s->bits)) +
                      static_cast<u32>(s->pitch) * static_cast<u32>(y) + depthBytes * static_cast<u32>(from);
        drawSpan<Bytes, Bytes == 3>(s, reinterpret_cast<u8 *>(static_cast<std::uintptr_t>(a)), Bytes,
                                    static_cast<u32>(n), flags, pixel, colour);
    };
    u32 d = 3 - 2 * static_cast<u32>(r);
    s32 x = 0, y = r;
    span(cy, sub(cx, r), add(add(cx, r), 1));
    for (;;) {
        if (static_cast<s32>(d) < 0) {
            d += 4 * static_cast<u32>(x) + 6;
        } else {
            span(sub(cy, y), sub(cx, x), add(add(cx, x), 1));
            span(add(cy, y), sub(cx, x), add(add(cx, x), 1));
            d += 4 * static_cast<u32>(sub(x, y)) + 10;
            --y;
        }
        if (++x >= y) break;
        span(sub(cy, x), sub(cx, y), add(add(cx, y), 1));
        span(add(cy, x), sub(cx, y), add(add(cx, y), 1));
    }
    if (x != y) return;
    span(sub(cy, x), sub(cx, y), add(add(cx, y), 1));
    span(add(cy, x), sub(cx, y), add(add(cx, y), 1));
}

template <u32 Bytes>
void drawText(const SurfaceInfo *s, const Rect *clip, u32 flags, s32 x, s32 y, const char *text, Color *colour) {
    if (!s || add(y, 6) > clip->bottom || y < clip->top || add(x, 6) > clip->right || !text || !*text) return;
    const u32 pixel = g_env->toPixel(colour, nullptr, reinterpret_cast<const PixelFormat *>(s));
    const bool blended = (flags & 0x18) != 0;
    const u32 src = source<Bytes>(pixel);
    const u32 srcMasked[3] = {s->redMask & src, s->greenMask & src, s->blueMask & src};
    const u32 alpha = alphaOf(colour, flags);
    // The 24-bit code steps 4 bytes a pixel (sic).
    const s32 step = Bytes == 3 ? 4 : static_cast<s32>(Bytes), startX = x;
    while (*text) {
        const s32 c = static_cast<s8>(*text++);
        u32 advance = 0;
        if (c == '\t') {
            x = add(x, 0x20) & ~0x1f;
        } else if (c == '\n') {
            y = add(y, 8);
            if (add(y, 6) > *g_env->textBottom) return;
            x = startX;
        } else {
            u32 index = static_cast<u32>(c) - 0x20;
            if (index > 0x88) index = 0;
            const u8 *glyph = g_env->font + index * 8;
            advance = glyph[6];
            if (x >= clip->left && add(x, 6) <= clip->right) {
                u8 *row = pixelAt<Bytes>(s, x, y);
                for (int r = 0; r < 6; ++r, row += s->pitch) {
                    u8 *q = row;
                    for (u32 bit = 0x40; bit >= 4; bit >>= 1, q += step) {
                        if (!(glyph[r] & bit)) continue;
                        store<Bytes>(q, blended ? blend(s, srcMasked, load<Bytes>(q), alpha) : pixel);
                    }
                }
            }
        }
        x = add(x, static_cast<s32>(advance));
    }
}

}  // namespace

void CanvasDraw_SetEnv(const CanvasDrawEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

#define CANVAS_DEPTH(N, BYTES)                                                                                     \
    void __fastcall Canvas##N##_Pixel(CanvasDriver *, void *, const SurfaceInfo *s, const Rect *clip, u32 flags,   \
                                      s32 x, s32 y, Color *colour) {                                               \
        drawPixel<BYTES>(s, clip, flags, x, y, colour);                                                            \
    }                                                                                                              \
    void __fastcall Canvas##N##_HSpan(CanvasDriver *, void *, const SurfaceInfo *s, const Rect *clip, u32 flags,   \
                                      s32 x, s32 y, s32 len, Color *colour) {                                      \
        drawHSpan<BYTES>(s, clip, flags, x, y, len, colour);                                                       \
    }                                                                                                              \
    void __fastcall Canvas##N##_VSpan(CanvasDriver *, void *, const SurfaceInfo *s, const Rect *clip, u32 flags,   \
                                      s32 x, s32 y, s32 len, Color *colour) {                                      \
        drawVSpan<BYTES>(s, clip, flags, x, y, len, colour);                                                       \
    }                                                                                                              \
    void __fastcall Canvas##N##_FillRect(CanvasDriver *, void *, const SurfaceInfo *s, const Rect *clip,           \
                                         u32 flags, const Rect *rect, Color *colour) {                             \
        fillRect<BYTES>(s, clip, flags, rect, colour);                                                             \
    }                                                                                                              \
    void __fastcall Canvas##N##_Line(CanvasDriver *d, void *, const SurfaceInfo *s, const Rect *clip, u32 flags,   \
                                     s32 x0, s32 y0, s32 x1, s32 y1, Color *colour) {                              \
        drawLine<BYTES>(d, s, clip, flags, x0, y0, x1, y1, colour);                                                \
    }                                                                                                              \
    void __fastcall Canvas##N##_Triangle(CanvasDriver *d, void *, const SurfaceInfo *s, const Rect *clip,          \
                                         u32 flags, s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2,               \
                                         Color *colour) {                                                          \
        drawTriangle<BYTES>(d, s, clip, flags, x0, y0, x1, y1, x2, y2, colour);                                    \
    }                                                                                                              \
    void __fastcall Canvas##N##_Mask(CanvasDriver *, void *, const SurfaceInfo *s, const Rect *clip, u32 flags,    \
                                     s32 x, s32 y, const CanvasMask *mask, Color *colour) {                        \
        drawMask<BYTES>(s, clip, flags, x, y, mask, colour);                                                       \
    }                                                                                                              \
    void __fastcall Canvas##N##_Sprite(CanvasDriver *, void *, const SurfaceInfo *s, const Rect *clip, u32 flags,  \
                                       const u32 *table, s32 x, s32 y, const CanvasMask *sprite) {                 \
        drawSprite<BYTES>(s, clip, flags, table, x, y, sprite);                                                    \
    }                                                                                                              \
    void __fastcall Canvas##N##_Circle(CanvasDriver *, void *, const SurfaceInfo *s, const Rect *clip, u32 flags,  \
                                       s32 x, s32 y, s32 radius, Color *colour) {                                  \
        drawCircle<BYTES>(s, clip, flags, x, y, radius, colour);                                                   \
    }                                                                                                              \
    void __fastcall Canvas##N##_Disc(CanvasDriver *, void *, const SurfaceInfo *s, const Rect *clip, u32 flags,    \
                                     s32 x, s32 y, s32 radius, Color *colour) {                                    \
        drawDisc<BYTES>(s, clip, flags, x, y, radius, colour);                                                     \
    }                                                                                                              \
    void __fastcall Canvas##N##_Text(CanvasDriver *, void *, const SurfaceInfo *s, const Rect *clip, u32 flags,    \
                                     s32 x, s32 y, const char *text, Color *colour) {                              \
        drawText<BYTES>(s, clip, flags, x, y, text, colour);                                                       \
    }
CANVAS_DEPTH(8, 1)
CANVAS_DEPTH(16, 2)
CANVAS_DEPTH(24, 3)
CANVAS_DEPTH(32, 4)
#undef CANVAS_DEPTH

void __fastcall Canvas_Outline(CanvasDriver *d, void *, const SurfaceInfo *s, const Rect *clip, u32 flags,
                               const Rect *rect, Color *colour) {
    Rect r = *rect;
    g_env->order(&r);
    if (!g_env->overlaps(&r, nullptr, clip)) return;
    const s32 across = sub(sub(r.right, r.left), 1), down = sub(sub(r.bottom, r.top), 1);
    reinterpret_cast<SpanFn>(d->table[2])(d, nullptr, s, clip, flags, r.left, r.top, across, colour);
    reinterpret_cast<SpanFn>(d->table[3])(d, nullptr, s, clip, flags, sub(r.right, 1), r.top, down, colour);
    reinterpret_cast<SpanFn>(d->table[2])(d, nullptr, s, clip, flags, add(r.left, 1), sub(r.bottom, 1), across,
                                          colour);
    reinterpret_cast<SpanFn>(d->table[3])(d, nullptr, s, clip, flags, r.left, add(r.top, 1), down, colour);
}

}  // extern "C"
