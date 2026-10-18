#include "stripanim.h"

#include <cstdint>

#include "../math/x87.h"
#include "../terrain/land.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

}  // namespace

extern "C" {

void __cdecl StripAnim_Apply(const u8 *object, float t) {
    if (!(at<u8>(object, 4) & 2)) return;
    const u8 *const shape = at<const u8 *>(object, 0xb4);
    const s32 n = *at<const s32 *>(object, 0xb0);
    float *const verts = at<float *>(shape, 0x10);
    u32 *const coords = at<u32 *>(at<const u8 *>(at<const u8 *>(at<const u8 *>(shape, 0x24), 4), 0x78), 0x68);
    if (n <= 0) return;
    // the y of vertex k, and component c (u, v) of vertex k's coordinates
    auto y = [verts](u32 k) -> float & { return verts[(k >> 2) * 12 + (k & 3) + 4]; };
    auto coord = [coords](u32 k, u32 c) -> u32 & { return coords[(k >> 2) * 8 + (k & 3) + 4 * c]; };
    const X87Reg shift = X87Reg(t) - 0.25f;
    const X87Reg count = (double)n;
    u32 j = 2;
    for (s32 i = 0; i < n; ++i, j += 4) {
        const X87Reg scale = i % 2 ? 0.8f : 1.0f;
        X87Reg v = X87_SubR((X87Reg((double)i) / count * 0.8f - shift) * 4.0f, X87Reg(1.0f));
        if (!(v.v >= 0.0)) v = 0.0f;  // fcom: below, or unordered
        else if (v.v > 1.0) v = 1.0f;
        const X87Reg w = scale * v;
        const X87Reg rise = w * 10.0f;
        y(j - 2) = (float)(X87Reg(y(j)) + rise);
        y(j - 1) = (float)(X87Reg(y(j + 1)) + rise);
        const float rest = (float)X87_SubR(w, X87Reg(1.0f));
        coord(j, 0) = 0;
        reinterpret_cast<float &>(coord(j, 1)) = rest;
        coord(j + 1, 0) = 0x3f800000;  // 1.0f
        reinterpret_cast<float &>(coord(j + 1, 1)) = rest;
    }
}

void __cdecl StripAnim_StartShrink(u8 *object) {
    if (!at<const u8 *>(object, 0xb0)) return;
    u32 &flags = at<u32>(object, 4);
    at<u32>(object, 0xbc) = 0xbe99999a;  // -0.3f
    if (flags & 0x20) flags = (flags & ~0x40u) | 0x80;
}

void __cdecl StripAnim_FitGround(const u8 *object) {
    if (!(at<u8>(object, 4) & 2)) return;
    const s32 count = *at<const s32 *>(object, 0xb0);
    const float base = at<float>(at<const u8 *>(at<const u8 *>(at<const u8 *>(object, 8), 4), 0x78), 0x44);
    const u8 *map = *reinterpret_cast<const u8 *const *>(0x007a0854);
    if (!map || !at<const u8 *>(map, 0x6c)) return;
    const u32 from = at<u32>(object, 0xc0), to = at<u32>(object, 0xc4);
    const float x0 = (float)(from >> 16), x1 = (float)(to >> 16), y0 = (float)(from & 0xffff), y1 = (float)(to & 0xffff);
    if (count <= 0) return;
    s32 i = 0;
    do {
        for (u32 k = 0; k < 2; ++k) {
            const u8 *strips = at<const u8 *>(object, 0xb0);
            const u32 end = at<const u32 *>(strips, 0xc)[2 * i + k];
            s32 cell[2] = {(s32)(end >> 16), (s32)(end & 0xffff)};
            const float v = (float)(X87Reg((double)(end & 0xffff)) / X87Reg((double)at<s32>(strips, 8)));
            const float u = (float)(X87Reg((double)(end >> 16)) / X87Reg((double)at<s32>(strips, 4)));
            Terrain_FootprintCell(at<s32>(at<const u8 *>(object, 8), 0x10), x0, y0, x1, y1, u, v, &cell[0], &cell[1]);
            const u8 *grid = at<const u8 *>(*reinterpret_cast<const u8 *const *>(0x007a0854), 0x6c);
            const X87Reg y = (X87Reg(at<const float *>(grid, 0x28)[(at<s32>(grid, 0x18) + 1) * cell[1] + cell[0]]) - base) * 0.5f;  // 0x6fe444
            const X87Reg top = y - -10.0f;                                                                                           // 0x6fe460
            const u32 low = 4 * (u32)i + k, high = low + 2;
            at<float *>(at<const u8 *>(object, 0xb4), 0x10)[(low & 3) + (low >> 2) * 12 + 4] = (float)top;
            at<float *>(at<const u8 *>(object, 0xb4), 0x10)[(high & 3) + (high >> 2) * 12 + 4] = (float)y;
        }
    } while (++i < count);
}
}
