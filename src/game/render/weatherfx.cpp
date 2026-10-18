#include "weatherfx.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../gfx/texture.h"
#include "../math/x87.h"
#include "../terrain/landscape.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const float kPi = 3.14159265f;           // 0x701c10
const float kPerDegree = 1.0f / 180.0f;  // 0x701c98

u32 Bits(float f) { return *reinterpret_cast<u32 *>(&f); }

// +0x331c = (x, -1, z) made unit length, unless its length is 0 or a NaN (fcom equal or unordered).
void SetDirection(u8 *fx, X87Reg x, X87Reg z) {
    const float xf = (float)x;
    const X87Reg xx = x * xf;
    const float zf = (float)z;
    const X87Reg zz = z * zf;
    const X87Reg length = X87_Sqrt(xx + zz - X87Reg(-1.0f));   // 0x701c9c
    u32 out[3] = {Bits(xf), 0xbf800000, Bits(zf)};
    if (length.v != 0.0 && length.v == length.v) {   // 0x701ca0
        const X87Reg scale = X87Reg(1.0f) / length;   // 0x701c68
        out[0] = Bits((float)(scale * xf));
        out[1] = Bits((float)(scale * X87Reg(-1.0f)));
        out[2] = Bits((float)(scale * zf));
    }
    at<u32>(fx, 0x331c) = out[0];
    at<u32>(fx, 0x3320) = out[1];
    at<u32>(fx, 0x3324) = out[2];
}

// The landscape's point i at (sin((angle - a) degrees) * strength, -1, cos((angle - b) degrees) * strength); its x and z.
void SetPoint(s32 i, float angle, float a, float b, float strength, float &x, float &z) {
    const X87Reg sine = X87_Sin(((X87Reg(angle) - a) * kPi * kPerDegree).v);
    const X87Reg cosine = X87_Cos(((X87Reg(angle) - b) * kPi * kPerDegree).v);
    x = (float)(sine * strength);
    z = (float)(cosine * strength);
    Landscape_SetPoint(reinterpret_cast<u8 *>((uintptr_t)0x008c1828), nullptr, i, Bits(x), 0xbf800000, Bits(z));
}

}  // namespace

extern "C" {

void __fastcall WeatherFx_SetDropCount(u8 *fx, void *, u32 count) { at<u32>(fx, 0x3318) = count; }

void __fastcall WeatherFx_SetFallSpeed(u8 *fx, void *, u32 speed) {
    u32 kinds = at<u32>(fx, 0);
    if (kinds & 1)
        at<u32>(fx, 0x3338) = speed;
    if (kinds & 2)
        at<u32>(fx, 0x333c) = speed;
}

void __fastcall WeatherFx_RequestReload(u8 *fx) { at<u32>(fx, 4) = 1; }

void __fastcall WeatherFx_SetWind(u8 *fx, void *, float angle, float strength) {
    const X87Reg t = X87Reg(angle) * kPi * kPerDegree;
    SetDirection(fx, X87Reg(X87_Cos(t.v)) * strength, X87Reg(X87_Sin(t.v)) * strength);
    const X87Reg turned = X87Reg(angle) - X87Reg(-270.0f);   // 0x701ca8 (kept in the register for point 0, stored for the rest)
    const float from = (float)turned;
    const float s = (float)(X87Reg(strength) * 0.0017f);   // 0x701ca4
    const X87Reg a = turned * kPi * kPerDegree;
    const float x0 = (float)(X87Reg(X87_Sin(a.v)) * s);
    const float z0 = (float)(X87Reg(X87_Cos(a.v)) * s);
    Landscape_SetPoint(reinterpret_cast<u8 *>((uintptr_t)0x008c1828), nullptr, 0, Bits(x0), 0xbf800000, Bits(z0));
    float x, z;
    SetPoint(1, from, 1.9f, 2.3f, s, x, z);     // 0x701cac, 0x701cb0
    SetPoint(2, from, 1.2f, -1.7f, s, x, z);    // 0x701cb4, 0x701cb8
    SetPoint(3, from, -1.8f, -2.9f, s, x, z);   // 0x701cbc, 0x701cc0
    SetDirection(fx, X87Reg(x) * 0.2f, X87Reg(z) * 0.2f);   // 0x701cc4
}

u32 __cdecl WeatherFx_Delete() {
    u8 *&slot = *reinterpret_cast<u8 **>((uintptr_t)0x008bcbac);
    if (!slot)
        return 0;
    WeatherFx_FreeTextures(slot);
    GameHeap_Free(slot);
    slot = nullptr;
    return 1;
}

u32 __fastcall WeatherFx_FreeTextures(u8 *fx) {
    for (u32 offset = 0x3348; offset <= 0x3350; offset += 4) {
        if (!at<u8 *>(fx, offset))
            continue;
        Texture_Release(at<u8 *>(fx, offset));
        if (u8 *const array = at<u8 *>(fx, offset))
            Texture_DestroyArray(array);
        at<u32>(fx, offset) = 0;
    }
    return 1;
}
}
