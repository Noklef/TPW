// UV animation of .md2 motion tracks: reimplementation of 0x4745c0.
#include "uvanim.h"

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
                const float f0 = (float)frames[k], f1 = (float)frames[j];
                t = (frame - f0) / (f1 - f0);
                break;
            }
        }
        const float w = 1.0f - t;  // 0x6feca8
        const float *key = uvs + 2 * k;
        const float u = key[0] * w + key[2] * t;
        const float v = key[1] * w + key[3] * t;
        float *block = blocks + 8 * (i >> 2);
        if (add) {
            block[i & 3] = u + block[i & 3];
            block[4 + (i & 3)] = v + block[4 + (i & 3)];
        } else {
            block[i & 3] = u;
            block[4 + (i & 3)] = v;
        }
    }
}

extern "C" void __cdecl Motion_ApplyUVs(const void *state, const void *track, s32 add) {
    const void *node = field<const void *>(track, 0x3c);
    Motion_UVsAt(field<const UvTrack *>(track, 0x2c), field<float>(state, 0x20), field<float *>(node, 0x68), add);
}
