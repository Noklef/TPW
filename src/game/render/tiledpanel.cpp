#include "tiledpanel.h"

#include <emmintrin.h>

#include <cstdint>

#include "batchquad.h"
#include "../math/x87.h"
#include "../gfx/texcache.h"
#include "fixedverts.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TiledPanelEnv kPortEnv = {&Batch_DrawQuad};
const TiledPanelEnv *g_env = &kPortEnv;

// The constants, as the game's memory holds them: 1, -1, -0.5, 0.5.
const float &kOne = global<float>(0x00702030);
const float &kLessOne = global<float>(0x00702038);
const float &kLessHalf = gTiledPanel_LessHalf_70204c;
const float &kHalf = gTiledPanel_Half_702050;

// a * b / 2, then fistp: round to nearest, through the dword 0xfb34a4.
s32 Pixel(X87Reg a, float b) {
    const float scaled = (float)((a * b) * kHalf);
    const s32 n = _mm_cvtss_si32(_mm_set_ss(scaled));
    gFixedVerts_fb34a4 = n;
    return n;
}

s32 Tile() { return at<s32>(reinterpret_cast<u8 * &>(gTexcache_8c4b44), 0x18f0); }

// The screen's y of the place: its top, by the anchor flags.
X87Reg AnchoredY(const u8 *object, const u8 *panel, u32 flags, float y, float screenH) {
    X87Reg top = X87Reg(-X87Reg(y).v);
    const s32 anchor = at<s16>(object, 0x32);
    if (flags & 0x7000000) {
        const X87Reg part = X87Reg(at<float>(object, 0x2c)) / X87Reg((double)at<u16>(panel, 0xe)) * X87Reg((double)anchor);
        const X87Reg twice = part + part;
        if (flags & 0x2000000)
            top = top - twice * kLessHalf;
        else if (flags & 0x4000000)
            top = top + twice;
    }
    const X87Reg pixel = (X87Reg(at<float>(object, 0x2c)) + X87Reg(at<float>(object, 0x2c))) / screenH;
    return top - X87Reg((double)anchor) * pixel;
}

// The x of the place, by the anchor flags.
float AnchoredX(const u8 *object, const u8 *panel, u32 flags, float x) {
    if (!(flags & 0x70000000))
        return x;
    const X87Reg part = X87Reg(at<float>(object, 0x2c)) / X87Reg((double)at<u16>(panel, 0xc)) * X87Reg((double)at<s16>(object, 0x30)) * global<float>(0x008bcbcc);
    const X87Reg twice = part + part;
    if (flags & 0x20000000)
        return (float)(x - twice * kHalf);
    if (flags & 0x40000000)
        return (float)(X87Reg(x) - twice);
    return x;
}

}  // namespace

void TiledPanel_SetEnv(const TiledPanelEnv *env) { g_env = env ? env : &kPortEnv; }

u32 __fastcall TiledPanel_Draw(u8 *object, void *) {
    const u8 *const panel = at<const u8 *>(object, 0x28);
    const s32 size = Tile();
    const s32 columns = ((s32)at<u16>(panel, 0x10) + size - 1) / size;
    const s32 rows = ((s32)at<u16>(panel, 0x12) + size - 1) / size;
    const u8 *cell = at<const u8 *>(panel, 0);
    const u32 flags = at<u32>(object, 0x10);
    const u32 zBits = at<u32>(object, 0x1c);

    u8 quad[0xb4] = {};
    u8 *const points = quad, *const spec = quad + 0x54;
    at<u32>(spec, 0) = 0x400;
    at<u8 *>(spec, 4) = points;
    at<u32>(spec, 8) = at<u32>(object, 0x24);
    at<u32>(spec, 0x28) = 2;

    const float screenH = (float)X87Reg((double)global<s32>(0x008bcbd4));
    const X87Reg top = AnchoredY(object, panel, flags, at<float>(object, 0x18), screenH);
    const float x = AnchoredX(object, panel, flags, at<float>(object, 0x14));
    const s32 tile = Tile();
    s32 pixelY = Pixel(top - kLessOne, screenH);
    const float toX = (float)(kOne / X87Reg((double)global<s32>(0x008bcbd0)));
    const float toY = (float)(kOne / X87Reg((double)global<s32>(0x008bcbd4)));
    if (rows <= 0)
        return 1;
    const float left = (float)(X87Reg(x) - kLessOne);
    for (s32 row = 0; row < rows; ++row, pixelY += tile) {
        s32 pixelX = Pixel(X87Reg((double)global<s32>(0x008bcbd0)), left);
        if (columns <= 0)
            continue;
        const float upper = (float)(X87Reg((double)(s32)((u32)pixelY * 2)) * toY - kOne);
        for (s32 column = 0; column < columns; ++column, pixelX += tile, cell += 0x60) {
            if (cell) {
                at<u32>(spec, 0x10) = at<u32>(cell, 8) & 0x1000 ? at<u32>(cell, 0x1c) : 0;
                at<float>(spec, 0x18) = (float)X87Reg(at<float>(cell, 0x28));
                at<float>(spec, 0x1c) = (float)X87Reg(at<float>(cell, 0x2c));
                at<float>(spec, 0x20) = (float)(X87Reg(at<float>(cell, 0x20)) + at<float>(spec, 0x18));
                at<float>(spec, 0x24) = (float)(X87Reg(at<float>(cell, 0x24)) + at<float>(spec, 0x1c));
                if (at<u32>(cell, 8) & 0x100000)
                    at<u32>(spec, 0) |= 4;
            } else {
                at<u32>(spec, 0x10) = 0;
                at<u32>(spec, 0x24) = 0;
                at<u32>(spec, 0x20) = 0;
                at<u32>(spec, 0x1c) = 0;
                at<u32>(spec, 0x18) = 0;
            }
            at<u32>(spec, 0xc) = 0x7060556;

            const s32 now = Tile();
            const u8 *const held = at<const u8 *>(object, 0x28);
            s32 width = (s32)at<u16>(held, 0x10) - column * now;
            s32 height = (s32)at<u16>(held, 0x12) - row * now;
            if (width > now)
                width = now;
            if (height > now)
                height = now;
            const float l = (float)(X87Reg((double)(s32)((u32)pixelX * 2)) * toX - kOne);
            const float r = (float)(X87Reg((double)(s32)((u32)(pixelX + width) * 2)) * toX - kOne);
            const float b = (float)(X87Reg((double)(s32)((u32)(pixelY + height) * 2)) * toY - kOne);
            const float xs[4] = {l, r, r, l}, ys[4] = {upper, upper, b, b};
            for (u32 k = 0; k < 4; ++k) {
                at<float>(points, k * 0x10) = xs[k];
                at<float>(points, k * 0x10 + 4) = ys[k];
                at<u32>(points, k * 0x10 + 8) = zBits;
                at<u32>(points, k * 0x10 + 0xc) = 0x3f800000;
            }
            if (!g_env->drawQuad(global<u8 *>(0x00879ab0), nullptr, spec, 0xffffffff))
                return 1;
        }
    }
    return 1;
}
