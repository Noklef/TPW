#include "texcoords.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, s32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

X87Reg Negate(X87Reg x) { return X87Reg(-x.v); }

}  // namespace

extern "C" {

void __cdecl Model_TexCoords(u8 *out, const u8 *settings, s32 index, const u8 *texture, s32 stride, s32 mode) {
    const u8 *const entry = at<const u8 *>(settings, 0x6c) + 16 * index;
    const u32 first = at<u16>(entry, 8);
    const u32 count = (u32)at<u16>(entry, 0xa) - first + 1;
    if (!texture || !(at<u32>(texture, 8) & 0x1000) || !at<u32>(texture, 0x1c)) {
        for (u32 n = count; n; --n, out += stride) {
            at<u32>(out, 0) = 0;
            at<u32>(out, 4) = 0;
        }
        return;
    }
    const float uOffset = at<float>(texture, 0x28);
    const float vOffset = at<float>(texture, 0x2c);
    const float uScale = at<float>(texture, 0x20);
    const float vScale = at<float>(texture, 0x24);
    if (mode == 1) {
        const u8 *const normals = at<const u8 *>(settings, 0x64);
        const float *const view = global<const float *>(0x0087afe8);
        const float a0 = view[0], a1 = view[1], a2 = view[2], b0 = view[4], b1 = view[5], b2 = view[6];
        for (u32 n = 0; n < count; ++n, out += stride) {
            const float *const normal = reinterpret_cast<const float *>(normals + 12 * at<const u16 *>(settings, 0x94)[first + n]);
            const X87Reg dotA = (X87Reg(normal[0]) * a0 + X87Reg(normal[1]) * a1) + X87Reg(normal[2]) * a2;
            at<float>(out, 0) = (float)((dotA - X87Reg(-1.0f)) * 0.5f * uScale + uOffset);   // 0x701d84, 0x701d88
            const X87Reg dotB = (X87Reg(normal[0]) * b0 + X87Reg(normal[1]) * b1) + X87Reg(normal[2]) * b2;
            at<float>(out, 4) = (float)(X87_SubR(dotB * 0.5f, X87Reg(0.5f)) * vScale + vOffset);
        }
        return;
    }
    const u8 *pack = at<const u8 *>(settings, 0x68) + (first >> 2) * 32;
    u32 lane = first & 3;
    if (mode & 2) {
        const float *const light = at<const float *>(global<const u8 *>(0x0087a248), 0x30);
        const float k = global<float>(0x008c1618);
        const float su = (float)(X87Reg(uScale) / X87Reg((s32)at<s16>(texture, 0x14)));
        const float sv = (float)Negate(X87Reg(vScale) / X87Reg((s32)at<s16>(texture, 0x16)));
        const float *edge = reinterpret_cast<const float *>(at<const u8 *>(settings, 0x74) + first * 24);
        for (u32 n = count; n; --n, out += stride) {
            const X87Reg s = (X87Reg(edge[2]) * light[2] + X87Reg(edge[1]) * light[1]) + X87Reg(edge[0]) * light[0];
            const X87Reg t = (X87Reg(edge[5]) * light[2] + X87Reg(edge[4]) * light[1]) + X87Reg(edge[3]) * light[0];
            edge += 6;
            const X87Reg sk = s * k;
            const X87Reg tk = t * k;
            const float limit = global<float>(0x008c161c);
            float shiftU = (float)sk;
            if (sk.v > limit)
                shiftU = limit;
            const float low = X87_Through(-limit);
            if (!(shiftU >= low))   // (below, or unordered)
                shiftU = low;
            X87Reg shiftV = tk;
            if (shiftV.v > limit)
                shiftV = X87Reg(limit);
            if (!(shiftV.v >= low))
                shiftV = X87Reg(low);
            const X87Reg u = (X87Reg(at<float>(pack, 4 * lane)) * uScale + X87Reg(su) * shiftU) + uOffset;
            const X87Reg dv = X87Reg(sv) * shiftV;
            at<float>(out, 0) = (float)u;
            at<float>(out, 4) = (float)((X87Reg(at<float>(pack, 0x10 + 4 * lane)) * vScale + dv) + vOffset);
            ++lane;
            pack += (lane >> 2) * 32;
            lane &= 3;
        }
        return;
    }
    for (u32 n = count; n; --n, out += stride) {
        at<float>(out, 0) = (float)(X87Reg(at<float>(pack, 4 * lane)) * uScale + uOffset);
        at<float>(out, 4) = (float)(X87Reg(at<float>(pack, 0x10 + 4 * lane)) * vScale + vOffset);
        ++lane;
        pack += (lane >> 2) * 32;
        lane &= 3;
    }
}
}
