// The canvas primitives' pixel, spans and outline: see canvasdraw.h.
#include "canvasdraw.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const CanvasDrawEnv kGameEnv = {
    at<u32(__fastcall *)(Color *, void *, const PixelFormat *)>(0x005fd050),
    at<void(__fastcall *)(Rect *)>(0x0062f940),
    at<u32(__fastcall *)(const Rect *, void *, const Rect *)>(0x0062f890),
};
const CanvasDrawEnv *g_env = &kGameEnv;

using SpanFn = void(__fastcall *)(CanvasDriver *, void *, const SurfaceInfo *, const Rect *, u32, s32, s32, s32,
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
