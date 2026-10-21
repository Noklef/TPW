#include "xaadpcm.h"

#include <cstdint>

namespace {

const s32 *coefficients1() { return reinterpret_cast<const s32 *>((uintptr_t)0x00705ec8); }
const s32 *coefficients2() { return reinterpret_cast<const s32 *>((uintptr_t)0x00705ed8); }

// (nibble << 28 >> shift) + a + b + 0x80 >> 8, the sum wrapping, clamped to 16 bits.
s32 sample(u32 shifted, s32 shift, u32 a, u32 b) {
    const s32 v = (s32)((u32)((s32)shifted >> shift) + a + b + 0x80) >> 8;
    return v < -0x8000 ? -0x8000 : v > 0x7fff ? 0x7fff : v;
}

}  // namespace

extern "C" {

s32 *__cdecl XaAdpcm_DecodeMono(s32 count, s32 *position, s32 *last, s32 *before, const u8 *data, s16 **out) {
    const u8 *src = data + (s32)((u32)*position * 15) / 28;
    s32 latest = *last, older = *before;
    s16 *to = *out;
    if (count > 0) {
        for (u32 blocks = ((u32)count + 0x1b) / 28; blocks; --blocks) {
            *position += 0x1c;
            const u8 header = *src++;
            const u32 c1 = (u32)coefficients1()[header >> 4], c2 = (u32)coefficients2()[header >> 4];
            const s32 shift = (header & 0xf) + 8;
            for (s16 *const end = to + 0x38; to < end; to += 4, ++src) {
                const s32 a = sample((u32)(*src >> 4) << 28, shift, (u32)older * c2, (u32)latest * c1);
                to[0] = to[1] = (s16)a;
                const s32 b = sample((u32)*src << 28, shift, (u32)a * c1, (u32)latest * c2);
                to[2] = to[3] = (s16)b;
                older = a;
                latest = b;
            }
        }
    }
    *last = latest;
    *before = older;
    *out = to;
    return before;
}

s32 __cdecl XaAdpcm_DecodeStereo(s32 count, s32 *position, s32 *last_left, s32 *before_left, s32 *last_right, s32 *before_right,
                                 const u8 *data, s16 **out) {
    const u8 *src = data + (s32)((u32)*position * 30) / 28;
    s32 left = *last_left, left_before = *before_left, right = *last_right, right_before = *before_right;
    s16 *to = *out;
    if (count > 0) {
        for (u32 blocks = ((u32)count + 0x1b) / 28; blocks; --blocks) {
            *position += 0x1c;
            const u8 filters = src[0], shifts = src[1];
            src += 2;
            const u32 c1l = (u32)coefficients1()[filters >> 4], c2l = (u32)coefficients2()[filters >> 4];
            const u32 c1r = (u32)coefficients1()[filters & 0xf], c2r = (u32)coefficients2()[filters & 0xf];
            const s32 shift_right = (shifts & 0xf) + 8, shift_left = (shifts >> 4) + 8;
            for (s16 *const end = to + 0x38; to < end; to += 4, src += 2) {
                const s32 l = sample((u32)(src[0] & 0xf0) << 24, shift_left, (u32)left * c1l, (u32)left_before * c2l);
                const s32 r = sample((u32)src[0] << 28, shift_right, (u32)right * c1r, (u32)right_before * c2r);
                to[0] = (s16)l;
                to[1] = (s16)r;
                const s32 l2 = sample((u32)(src[1] & 0xf0) << 24, shift_left, (u32)l * c1l, (u32)left * c2l);
                const s32 r2 = sample((u32)src[1] << 28, shift_right, (u32)right * c2r, (u32)r * c1r);
                to[2] = (s16)l2;
                to[3] = (s16)r2;
                left_before = l;
                right_before = r;
                left = l2;
                right = r2;
            }
        }
    }
    *last_left = left;
    *before_left = left_before;
    *last_right = right;
    *before_right = right_before;
    *out = to;
    return right_before;
}
}
