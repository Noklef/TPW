#include "mat4.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <initializer_list>

#include "x87.h"
#include "../render/viewport.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// a / b rounded once to a 24-bit significand, for an a of up to 53 bits (fsin's result): the double quotient can round onto a 24-bit
// midpoint, and its exact remainder (fma) says which side of it the true quotient is on.
double DivRound(double a, double b) {
    if (a != a || b != b) return (X87Reg(a) / X87Reg(b)).v;
    const double q = a / b;
    if (!std::isfinite(q) || q == 0) return q;
    int e;
    const double frac = std::frexp(std::fabs(q), &e);
    const unsigned long long m = (unsigned long long)std::ldexp(frac, 53);
    if ((m & ((1ull << 29) - 1)) != (1ull << 28)) return X87_Round(q);
    const double rem = std::fma(-q, b, a);
    if (rem == 0) return X87_Round(q);
    const bool above = (rem > 0) == (b > 0);  // a / b beyond q (towards +infinity)
    const double r = std::ldexp((double)((m >> 29) + (above == (q > 0) ? 1 : 0)), e - 24);
    return q < 0 ? -r : r;
}

}  // namespace

extern "C" {

void __cdecl Mat4_InitGlobals() {
    for (u32 address : {0x87b090u, 0x87b110u, 0x87aff0u, 0x87b030u, 0x87af68u, 0x87afa8u, 0x87b158u, 0x87b198u})
        Mat4_Identity(reinterpret_cast<u32 *>((uintptr_t)address));
}

void __fastcall Mat4_Identity(u32 *m) {
    for (int i = 0; i < 16; ++i)
        m[i] = 0;
    m[15] = m[10] = m[5] = m[0] = 0x3f800000;
}

void __fastcall Mat4_Assign(u32 *m, void *, const u32 *from) {
    for (int i = 0; i < 16; ++i)
        m[i] = from[i];
}

void __fastcall Mat4_MoveAlongX(float *m, void *, float distance) {
    const X87Reg x = X87Reg(distance) * m[0] + m[12];
    const X87Reg y = X87Reg(m[1]) * distance + m[13];
    const X87Reg z = X87Reg(m[2]) * distance + m[14];
    const X87Reg w = X87Reg(m[3]) * distance + m[15];
    m[12] = (float)x;
    m[13] = (float)y;
    m[14] = (float)z;
    m[15] = (float)w;
}

void __cdecl Mat4_LuBackSubstitute(const float *lu, const s32 *index, float *b) {
    u32 *bits = reinterpret_cast<u32 *>(b);
    s32 first = -1;
    for (s32 i = 0; i < 4; ++i) {
        const s32 ip = index[i];
        const float picked = b[ip];
        bits[ip] = bits[i];
        if (first >= 0) {
            X87Reg sum = picked;
            for (s32 j = first; j <= i - 1; ++j)
                sum = sum - X87Reg(b[j]) * lu[i * 4 + j];
            b[i] = (float)sum;
        } else {
            if (picked != 0 && picked == picked) first = i;
            b[i] = X87_Through(picked);
        }
    }
    for (s32 i = 3; i >= 0; --i) {
        X87Reg sum = b[i];
        for (s32 j = i + 1; j < 4; ++j)
            sum = sum - X87Reg(lu[i * 4 + j]) * b[j];
        b[i] = (float)(sum / lu[i * 5]);
    }
}

void __fastcall Mat4_Perspective(float *m, void *, float near_, float far_, float fov) {
    const X87Reg angle = X87Reg(fov) * 0.0174532924f;  // 0x7018b8
    const X87Reg ratio = X87Reg(near_) / far_;
    std::memset(m, 0, 16 * sizeof(float));
    const X87Reg half = angle * 0.5;  // 0x7018c0 (a double)
    const X87Reg depth = X87_SubR(ratio, 1.0f);
    const double s = X87_Sin(half.v), c = X87_Cos(half.v);
    const X87Reg q = DivRound(s, depth.v);
    const X87Reg shift = q * near_;
    m[0] = (float)c;
    m[5] = (float)c;
    m[10] = (float)q;
    m[14] = (float)(-shift.v);
    if (global<u32>(0x008bcbc8) & 0x200)
        m[15] = 1.0f;
    else
        m[11] = (float)s;
    std::memcpy(&global<u32>(0x0087b1dc), &near_, 4);
    std::memcpy(&global<u32>(0x00768acc), &far_, 4);
    std::memcpy(&global<u32>(0x0087af58), &fov, 4);
}

void __fastcall Mat4_SetFar(float *m, void *, u32 far_) {
    // The kept values go on as they are (as dwords, a signalling NaN too).
    using PerspectiveBits = void(__fastcall *)(float *, void *, u32, u32, u32);
    reinterpret_cast<PerspectiveBits>(&Mat4_Perspective)(m, nullptr, global<u32>(0x0087b1dc), far_, global<u32>(0x0087af58));
    float far_value;
    std::memcpy(&far_value, &far_, 4);
    const float near_ = global<float>(0x0087b1dc);
    X87Reg d = ((X87Reg(far_value) - m[14]) / m[10]) * global<float>(0x008bcc0c);
    if (!(d.v >= near_)) d = near_;
    const X87Reg b = X87Reg(m[11]) * d + m[15];
    const X87Reg a = d * m[10] + m[14];
    View_SetProjection(reinterpret_cast<u8 *>(0x008bcbc8), nullptr, far_, (float)a, (float)b);
}
}
