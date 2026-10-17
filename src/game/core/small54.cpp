#include "small54.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// byte += ((target - byte) * weight) >> 8, as the compiled code: 32-bit arithmetic cut to the low byte.
u8 Blend(u8 byte, u32 target, u32 weight) { return (u8)(((target - byte) * weight) >> 8) + byte; }

}  // namespace

extern "C" {

void __fastcall Pixels_Stamp(u8 *source, void *, u8 *destination, s32 x, s32 y) {
    const u8 *from = at<u8 *>(source, 0xc);
    u8 *to = at<u8 *>(destination, 0xc) + ((at<s32>(destination, 0) * y + x) * at<s32>(destination, 8));
    for (s32 row = 0; row < at<s32>(source, 4); ++row) {
        for (s32 column = 0; column < at<s32>(source, 0); ++column) {
            const s32 pixel = at<s32>(source, 8);
            if (pixel == 1) {
                const u32 v = from[0];
                to[0] = Blend(to[0], v, v);
                to[1] = Blend(to[1], from[0], v);
                to[2] = Blend(to[2], from[0], v);
                to[3] = Blend(to[3], from[0], v);
                ++from;
            } else if (pixel == 2) {
                to[0] = 0xff;
                to[1] = from[0];
                to[2] = from[1];
                to[3] = 0;
                from += 2;
            } else {
                const u32 alpha = from[0];
                to[0] = from[0] > to[0] ? from[0] : to[0];
                to[1] = Blend(to[1], from[1], alpha);
                to[2] = Blend(to[2], from[2], alpha);
                to[3] = Blend(to[3], from[3], alpha);
                from += 4;
            }
            to += 4;
        }
        to += (at<s32>(destination, 0) - at<s32>(source, 0)) * at<s32>(destination, 8);
    }
}

void __fastcall Mask_Tint(u8 *mask, void *, u8 *destination, u32 scale, u32 c1, u32 c2, u32 c3) {
    const u8 *from = at<u8 *>(mask, 0xc);
    if (!from) return;
    u8 *to = at<u8 *>(destination, 0xc);
    const u32 s = scale & 0xff;
    const u8 t1 = (u8)(((c1 & 0xff) * s) >> 8), t2 = (u8)(((c2 & 0xff) * s) >> 8), t3 = (u8)(((c3 & 0xff) * s) >> 8);
    for (s32 row = 0; row < at<s32>(mask, 4); ++row) {
        for (s32 column = 0; column < at<s32>(mask, 0); ++column) {
            const u8 a = (u8)((*from++ * s) >> 8);
            to[0] = a > to[0] ? a : to[0];
            to[1] = Blend(to[1], t1, a);
            to[2] = Blend(to[2], t2, a);
            to[3] = Blend(to[3], t3, a);
            to += 4;
        }
        to += (at<s32>(destination, 0) - at<s32>(mask, 0)) * at<s32>(destination, 8);
    }
}

}  // extern "C"
