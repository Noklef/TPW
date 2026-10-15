#include "keys.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

const float &start() { return *reinterpret_cast<const float *>(0x0079240c); }

}  // namespace

extern "C" {

u32 __cdecl Keys_MergeSamples(const u8 *a, const u8 *b, const float *keys, float *out, float span) {
    const u8 *key = reinterpret_cast<const u8 *>(keys);
    while (!(at<float>(key, 0) >= start())) key += 0x10;
    const u32 n = a == b ? *at<const u32 *>(b, 0x90) : 11;
    u32 written = 0;
    if (n == 0) return 0;
    const X87Reg last = (double)(n - 1);
    for (u32 i = 0; i < n; ++i) {
        const float from = (float)(X87Reg((double)i) / last * span + start());
        const float to = (float)(X87Reg((double)(i + 1)) / last * span + start());
        out[written++] = from;
        if (i >= n - 1) continue;
        for (;; key += 0x10) {
            const float k = at<float>(key, 0);
            if (!(k >= from) || k >= to) break;  // a NaN sample ends it, a NaN next does not
            if (!(k == from)) at<u32>(out, 4 * written++) = at<u32>(key, 0);
        }
    }
    return written;
}
}
