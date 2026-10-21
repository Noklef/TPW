#include "tqivideo.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// value * factor, 16.16, rounded by the product's bit 15 (IMUL; SHL EDX,16; SHR EAX,16; ADC).
s32 fixmul(s32 value, s32 factor) {
    const int64_t product = (int64_t)value * factor;
    const u32 low = (u32)product, high = (u32)((uint64_t)product >> 32);
    return (s32)((high << 16) + (low >> 16) + (low >> 15 & 1));
}

s32 ftol(X87Reg x) { return (s32)(long long)x.v; }   // __ftol (0x67a830): truncated, its low dword

s32 idct_scale(u32 i) { return global<s32>(0x00705c88 + i * 4); }

}  // namespace

extern "C" {

void __cdecl TqiVideo_SetQuality(s32 *out, s32 quality) {
    if (quality == 100) {
        for (u32 i = 0; i < 0x40; ++i) out[i] = fixmul(idct_scale(i), 0x80000);
    } else {
        const float scale = (float)(X87_SubR(X87Reg((double)quality), X87Reg(107.0)) * X87Reg(0.0625f));
        for (u32 i = 0; i < 0x40; ++i) {
            const s32 factor = ftol(X87Reg((double)global<s32>(0x00705d88 + i * 4)) * X87Reg(scale) * X87Reg(65536.0f));
            out[i] = fixmul(idct_scale(i), factor);
        }
    }
    out[0] = fixmul(idct_scale(0), 0x80000);
}
}
