#include "staff.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The tail both drains share: fcom with 100.0f ([0x70082c]), then 0.0f ([0x700830]).
void clamp_store(float &to, X87Reg v) {
    if (v.v > 100.0) {
        to = 100.0f;
    } else if (!(v.v >= 0.0)) {  // below 0, or unordered
        to = 0.0f;
    } else {
        to = (float)v;
    }
}

}  // namespace

extern "C" {

u32 __fastcall Staff_IsTooTired(const u8 *staff) {
    const u32 level = (u32)(long long)at<float>(staff, 0x1fc) & 0xff;  // __ftol
    return (s32)level < *reinterpret_cast<const s32 *>(0x00785324);
}

void __fastcall Staff_TireA(u8 *staff) {
    float &energy = at<float>(staff, 0x1fc);
    clamp_store(energy, X87Reg(-X87_MulIntDouble(6 - at<s32>(staff, 0x1e4), 0.012)) + X87Reg(energy));
    float &happiness = at<float>(staff, 0x1f8);
    clamp_store(happiness, X87Reg(-X87_MulIntDouble(6 - at<s32>(staff, 0x1e4), 0.005)) + X87Reg(happiness));
}

void __fastcall Staff_TireB(u8 *staff) {
    float &energy = at<float>(staff, 0x1fc);
    clamp_store(energy, X87Reg(-X87_MulIntDouble(6 - at<s32>(staff, 0x1e4), 0.025f)) + X87Reg(energy));
    float &happiness = at<float>(staff, 0x1f8);
    const X87Reg was = happiness;  // loaded before the drain
    const float drain = (float)-X87_MulIntDouble(6 - at<s32>(staff, 0x1e4), 0.01);
    clamp_store(happiness, was + X87Reg(drain));
}

void __fastcall Staff_SetName(u8 *staff, void *, const u16 *name) {
    // The CRT's wcsncpy (0x67b0d0), a unit at a time: up to and with the NUL, then zeros.
    u16 *to = &at<u16>(staff, 0x1a0);
    u32 count = 0x20;
    while (count && (*to++ = *name++) != 0) --count;
    if (count)
        while (--count) *to++ = 0;
    at<u16>(staff, 0x1e0) = 0;
}

u16 *__fastcall Staff_GetName(u8 *staff) { return &at<u16>(staff, 0x1a0); }
}
