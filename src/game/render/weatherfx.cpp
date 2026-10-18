#include "weatherfx.h"

#include <cstdint>
#include <cstdlib>

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

// The weather's random numbers (inline in the original): the renderer's seed at +0xa198 stepped.
u32 NextRandom(u8 *fx) {
    const u32 seed = (u32)_rotl(at<u32>(fx, 0xa198) * 0x13d573u, 3) + 0xe0423u;
    at<u32>(fx, 0xa198) = (u32)_rotl(seed, 4);
    return at<u32>(fx, 0xa198);
}

// Not 0 and not a NaN (fcom against 0: neither equal nor unordered).
bool NonZero(float x) { return x < 0.0f || x > 0.0f; }

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

float *__cdecl WeatherFx_BoltMidpoint(float *out, const float *a, const float *b, float scale) {
    float m[3], d[3];
    for (u32 i = 0; i < 3; ++i)
        m[i] = (float)((X87Reg(a[i]) + b[i]) * 0.5f);   // 0x701c84
    for (u32 i = 0; i < 3; ++i)
        d[i] = (float)(X87Reg(b[i]) - a[i]);
    const float q = (float)((X87Reg(m[0]) * m[0] + X87Reg(m[1]) * m[1]) + X87Reg(m[2]) * m[2]);
    u8 *const fx = *reinterpret_cast<u8 **>((uintptr_t)0x008bcbac);
    const float step = 1.0f / 8192;   // 0x701cc8
    const float r1 = (float)(X87Reg((s32)NextRandom(fx) % 0x1fff) * step);
    float r = (float)((X87Reg(r1) * r1) * (X87Reg((s32)NextRandom(fx) % 0x1fff) * step));
    if (NextRandom(fx) & 1)
        r = -r;
    NextRandom(fx);
    float p[3] = {m[0], m[1], m[2]};
    static const u8 kOrder[3][3] = {{0, 1, 2}, {1, 2, 0}, {2, 0, 1}};
    const s32 pick = (s32)NextRandom(fx) % 3;
    const u8 *const order = kOrder[pick == 0 ? 0 : pick == 1 ? 1 : 2];
    const u32 axis = NonZero(d[order[0]]) ? order[0] : NonZero(d[order[1]]) ? order[1] : order[2];
    p[axis] = (float)X87_SubR(X87Reg(q) / d[axis], m[axis]);
    const X87Reg n0 = X87Reg(p[2]) * d[1] - X87Reg(p[1]) * d[2];
    const float n1 = (float)(X87Reg(p[0]) * d[2] - X87Reg(p[2]) * d[0]);
    const float n2 = (float)(X87Reg(p[1]) * d[0] - X87Reg(p[0]) * d[1]);
    const X87Reg along = X87_Sqrt((X87Reg(d[0]) * d[0] + X87Reg(d[1]) * d[1]) + X87Reg(d[2]) * d[2]) * scale * r;
    const X87Reg f = along / X87_Sqrt((X87Reg(p[0]) * p[0] + X87Reg(p[1]) * p[1]) + X87Reg(p[2]) * p[2]);
    for (u32 i = 0; i < 3; ++i)
        p[i] = (float)(f * p[i]);
    const X87Reg g = X87_DivR(X87_Sqrt((n0 * n0 + X87Reg(n1) * n1) + X87Reg(n2) * n2), X87Reg(0.0f));   // 0x701ca0 (sic)
    out[0] = (float)((g * n0 + p[0]) + m[0]);
    const float gf = (float)g;
    out[1] = (float)((X87Reg(gf) * n1 + p[1]) + m[1]);
    out[2] = (float)((X87Reg(gf) * n2 + p[2]) + m[2]);
    return out;
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
