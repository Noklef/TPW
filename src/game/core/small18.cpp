#include "small18.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall Blocks7_Ctor(u8 *object) {
    for (u32 offset = 0; offset < 0x70; offset += 4)
        at<u32>(object, offset) = 0;
    return object;
}

void __fastcall Pixels_BlendOnto(const u8 *source, void *, u8 *dest, u32 x, u32 y) {
    u8 *out = at<u8 *>(dest, 0xc) + (at<u32>(dest, 0) * y + x) * at<u32>(dest, 8);
    const u8 *in = at<const u8 *>(source, 0xc);
    for (u32 row = 0; row < at<u32>(source, 4); ++row) {
        for (u32 column = 0; column < at<u32>(source, 0); ++column) {
            const s32 alpha = in[0];
            out[0] = 0xff;
            for (int channel = 1; channel < 4; ++channel) {
                const s32 d = out[channel];
                const s32 s = in[channel];
                out[channel] = (u8)(d + (((s - d) * alpha) >> 8));
            }
            in += 4;
            out += 4;
        }
        out += (at<u32>(dest, 0) - at<u32>(source, 0)) * at<u32>(dest, 8);
    }
}

}  // extern "C"
