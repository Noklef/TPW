#include "stripanim.h"

#include <cstdint>

#include "../math/x87.h"

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
}
