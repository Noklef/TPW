#include "aaline.h"

#include <cmath>
#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

bool Inside(const s32 *clip, s32 x, s32 y) { return clip[0] <= x && x < clip[2] && clip[1] <= y && y < clip[3]; }

// One pixel: mask 3 (alpha) is read before the clip test, the other masks as the blend needs them.
template <int N> void Blend(const SurfaceInfo *s, const s32 *clip, s32 x, s32 y, X87Reg distance, u8 *p, u32 colour) {
    const u32 keep = s->alphaMask;
    const double away = std::fabs(distance.v);
    if (!Inside(clip, x, y) || away > 1.0)
        return;
    const X87Reg level = (X87Reg(1.0) - X87Reg(away)) * X87Reg(255.0);
    const u32 a = level.v != level.v ? 0 : (u32)(u8)(long long)level.v;   // (__ftol: a NaN gives the indefinite, low byte 0)
    const u32 b = 0xff - a;
    const u32 old = at<u32>(p, 0);
    const u32 r = (s->redMask & old) * b + (s->redMask & colour) * a;
    const u32 g = (s->greenMask & old) * b + (s->greenMask & colour) * a;
    const u32 bl = (s->blueMask & old) * b + (s->blueMask & colour) * a;
    const u32 pixel = (((g >> 8) + g + 1) >> 8 & s->greenMask) | (((r >> 8) + r + 1) >> 8 & s->redMask) | (((bl >> 8) + bl + 1) >> 8 & s->blueMask) |
                      (keep & old);
    for (int k = 0; k < N; ++k)
        p[k] = (u8)(pixel >> (8 * k));
}

template <int N> void Draw(const SurfaceInfo *s, const s32 *clip, Color *colour, const s32 *from, const s32 *to) {
    s32 x0 = from[0], y0 = from[1], x1 = to[0], y1 = to[1];
    s32 dx = (s32)((u32)x1 - (u32)x0), dy = (s32)((u32)y1 - (u32)y0);
    const s32 adx = dx < 0 ? (s32)(0u - (u32)dx) : dx, ady = dy < 0 ? (s32)(0u - (u32)dy) : dy;
    s32 sx = dx >= 0 ? 1 : -1, sy = dy >= 0 ? 1 : -1, sxb = sx * N;
    const u32 col = Color_ToPixel(colour, nullptr, reinterpret_cast<const PixelFormat *>(s));
    s32 pitch = s->pitch;
    if (dy < 0)
        pitch = (s32)(0u - (u32)pitch);
    if (adx >= ady) {
        if (dx < 0) {
            const s32 t = x0;
            x0 = x1;
            x1 = t;
            y0 = y1;   // (sic: the end's y is not kept, and not needed)
            dx = adx;
            dy = (s32)(0u - (u32)dy);
            sx = -sx;
            sy = -sy;
            sxb = -sxb;
            pitch = (s32)(0u - (u32)pitch);
        }
        const s32 n = (s32)((u32)dx * (u32)dx + (u32)dy * (u32)dy);
        const u32 minor2 = 2u * (u32)ady, diag2 = ((u32)ady - (u32)dx) * 2u;
        u32 err = minor2 - (u32)dx;
        u8 *p = reinterpret_cast<u8 *>((u32)y0 * (u32)s->pitch + (uintptr_t)s->bits + (u32)x0 * N);
        u8 *pp = p + pitch, *pm = p - pitch;
        u32 d = 0;
        s32 x = x0, y = y0;
        const X87Reg root = X87_Sqrt(X87Reg((double)n));
        const X87Reg inv = X87_DivR(root + root, X87Reg(1.0));
        X87Reg w = X87_MulIntDouble(dx, inv.v);
        w = w + w;
        if (x == x1)
            return;
        do {
            const X87Reg dist = X87_MulIntDouble((s32)d, inv.v);
            Blend<N>(s, clip, x, y, dist, p, col);
            const s32 yp = y + sy;
            Blend<N>(s, clip, x, yp, w - dist, pp, col);
            Blend<N>(s, clip, x, y - sy, w + dist, pm, col);
            u32 step;
            if ((s32)err < 0) {
                d = err + (u32)dx;
                err += minor2;
                step = (u32)sxb;
            } else {
                d = err - (u32)dx;
                err += diag2;
                y = yp;
                step = (u32)pitch + (u32)sxb;
            }
            x += sx;
            p += step;
            pp += step;
            pm += step;
        } while (x != x1);
        return;
    }
    if (dy < 0) {
        x0 = x1;   // (sic: the start's x is the end's, and the end's is kept)
        const s32 t = y0;
        y0 = y1;
        y1 = t;
        sy = -sy;
        sx = -sx;
        dx = (s32)(0u - (u32)dx);
        sxb = -sxb;
        pitch = (s32)(0u - (u32)pitch);
    }
    const s32 n = (s32)((u32)dx * (u32)dx + (u32)ady * (u32)ady);
    const u32 major2 = 2u * (u32)adx, diag2 = ((u32)adx - (u32)ady) * 2u;
    u32 err = major2 - (u32)ady;
    u8 *p = reinterpret_cast<u8 *>((u32)y0 * (u32)s->pitch + (uintptr_t)s->bits + (u32)x0 * N);
    u8 *pp = p + sxb, *pm = p - sxb;
    u32 d = 0;
    s32 x = x0, y = y0;
    const X87Reg root = X87_Sqrt(X87Reg((double)n));
    const X87Reg inv = X87_DivR(root + root, X87Reg(1.0));
    X87Reg w = X87_MulIntDouble(ady, inv.v);
    w = w + w;
    if (y == y1)
        return;
    do {
        const X87Reg dist = X87_MulIntDouble((s32)d, inv.v);
        Blend<N>(s, clip, x, y, dist, p, col);
        const s32 xp = x + sx;
        Blend<N>(s, clip, xp, y, w - dist, pp, col);
        Blend<N>(s, clip, x - sx, y, w + dist, pm, col);
        u32 step;
        if ((s32)err < 0) {
            d = err + (u32)ady;
            err += major2;
            step = (u32)pitch;
        } else {
            d = err - (u32)ady;
            err += diag2;
            x = xp;
            step = (u32)pitch + (u32)sxb;
        }
        y += sy;
        p += step;
        pp += step;
        pm += step;
    } while (y != y1);
}

}  // namespace

extern "C" {

void __cdecl AaLine_Draw8(void *, const SurfaceInfo *surface, const s32 *clip, Color *colour, const s32 *from, const s32 *to) {
    Draw<1>(surface, clip, colour, from, to);
}

void __cdecl AaLine_Draw16(void *, const SurfaceInfo *surface, const s32 *clip, Color *colour, const s32 *from, const s32 *to) {
    Draw<2>(surface, clip, colour, from, to);
}

void __cdecl AaLine_Draw24(void *, const SurfaceInfo *surface, const s32 *clip, Color *colour, const s32 *from, const s32 *to) {
    Draw<3>(surface, clip, colour, from, to);
}

void __cdecl AaLine_Draw32(void *, const SurfaceInfo *surface, const s32 *clip, Color *colour, const s32 *from, const s32 *to) {
    Draw<4>(surface, clip, colour, from, to);
}
}
