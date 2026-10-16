#include "small41.h"

#include <cstdint>

#include "selectors.h"
#include "../world/thingscan.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// Bits 16 to 47 of the 64-bit product.
s32 Shifted(s32 a, s32 b) { return (s32)(u32)((uint64_t)((int64_t)a * (int64_t)b) >> 16); }

}  // namespace

extern "C" {

void __fastcall Matrix2_Transform(u8 *matrix, void *, s32 *out, const s32 *in) {
    const s32 first = Shifted(in[0], at<s32>(matrix, 0x24));
    const s32 second = Shifted(in[1], at<s32>(matrix, 0x2c));
    const s32 third = Shifted(in[0], at<s32>(matrix, 0x20));
    const s32 fourth = Shifted(in[1], at<s32>(matrix, 0x28));
    out[1] = second + first;
    out[0] = fourth + third;
}

u32 __fastcall Table_ScaledBySum(u8 *table, void *, s32 index) {
    const u32 total = (u32)Global7852f8_Times78548c();
    if (total == 0) return 0;
    const u32 share = (Things_SumChain() * 100u) / total;
    return (at<u32>(table, ((u32)index + 0x141) * 16) * share) / 100u;
}

}  // extern "C"
