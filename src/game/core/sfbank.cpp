#include "sfbank.h"

#include <cstdint>
#include <xmmintrin.h>

#include "../math/x87.h"
#include "sfman.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

s32 __fastcall SoundResource_LengthMs(u8 *self, void *, u32 a) {
    u8 buffer[0x30];   // (not cleared, as the original's)
    u8 *format = buffer;
    u32 answer;
    const s32 *result = vcall<const s32 *>(self, 0x18, &answer, &format, a);
    if (*result < 0) return 0;
    X87Reg length = (double)at<s32>(format, 4);   // fild
    if (at<u8>(format, 0x1b) & 1) length = length * global<float>(0x00711ba0);
    if (at<u8>(format, 0x1a) == 0x10) length = length * global<float>(0x00711ba0);
    const X87Reg rate = (double)at<u16>(format, 0x18);
    const float ms = (float)((length / rate) * global<float>(0x00711ba4));   // fdivr st0, st1; fmul; fstp
    return _mm_cvtss_si32(_mm_set_ss(ms));   // fld; fistp
}

u32 *__fastcall SfBank_Load(u8 *self, void *, u32 *out, u32 path, u32) {
    if (at<u8>(self, 0x18) & 1) {
        *out = 0;
        return out;
    }
    if (u8 *const synth = at<u8 *>(self, 0x14)) {
        u32 answer;
        if ((s32)*SfMan_LoadBank(synth, nullptr, &answer, path, 0) >= 0) {
            at<u32>(self, 0x18) |= 1;
            *out = 0;
            return out;
        }
    }
    *out = 0xffffffff;
    return out;
}

void __fastcall SfBank_Unload(u8 *self) {
    if (u8 *const synth = at<u8 *>(self, 0x14)) {
        SfMan_UnloadBank(synth);
        at<u32>(self, 0x14) = 0;
        at<u32>(self, 0x18) = 0;
    }
}

}  // extern "C"
