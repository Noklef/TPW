// UV animation of .md2 motion tracks: reimplementation of 0x4745c0.
#include "uvanim.h"

#include "../math/x87.h"

namespace {

template <typename T>
T field(const void *base, u32 offset) {
    return *reinterpret_cast<const T *>(static_cast<const u8 *>(base) + offset);
}

}  // namespace

void Motion_UVsAt(const UvTrack *track, float frame, float *blocks, s32 add) {
    for (u32 i = 0; i < track->count; ++i) {
        const UvRange range = track->ranges[i];
        const u16 *frames = track->frames + range.first;
        const float *uvs = track->uvs + 2 * range.first;
        const u32 whole = (u32)(long long)frame;  // __ftol: 64-bit truncation, low half
        s32 k = (s32)range.keys - 2;
        float t = 1.0f;
        for (u32 j = 0; j < range.keys; ++j) {
            if (whole < frames[j]) {
                k = (s32)j - 1;
                const X87Reg f0 = (double)frames[k], f1 = (double)frames[j];  // fild
                t = (float)((X87Reg(frame) - f0) / (f1 - f0));
                break;
            }
        }
        // In the x87's order (its NaNs come through as it picks them): w = 1 - t (0x6feca8) in a
        // register, u = u1 * t + u0 * w and v = v1 * t + v0 * w, the t product the destination.
        const X87Reg w = X87Reg(1.0f) - X87Reg(t);
        const float *key = uvs + 2 * k;
        X87Reg u = X87Reg(key[2]) * X87Reg(t) + X87Reg(key[0]) * w;
        X87Reg v = X87Reg(key[3]) * X87Reg(t) + X87Reg(key[1]) * w;
        float *block = blocks + 8 * (i >> 2);
        if (add) {
            u = u + X87Reg(block[i & 3]);
            v = v + X87Reg(block[4 + (i & 3)]);
        }
        block[i & 3] = (float)u;
        block[4 + (i & 3)] = (float)v;
    }
}

extern "C" void __cdecl Motion_ApplyUVs(const void *state, const void *track, s32 add) {
    const void *node = field<const void *>(track, 0x3c);
    Motion_UVsAt(field<const UvTrack *>(track, 0x2c), field<float>(state, 0x20), field<float *>(node, 0x68), add);
}
