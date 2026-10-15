#include "helpers.h"

#include <cmath>

#include "../math/x87.h"

namespace {

const float kPi = 3.14159274f;     // 0x6fe120
const float kTwoPi = 6.28318548f;  // 0x6fe124
const double kMinusPiD = -3.1415927410125732;  // 0x6fe128
const double kTwoPiD = 6.2831854820251465;     // 0x6fe130

}  // namespace

extern "C" {

int __cdecl Float_RoundUp(float x) {
    double whole;
    const double frac = std::modf((double)x, &whole);
    int n = (int)(long long)whole;
    if (frac < 0.0 || frac > 0.0) ++n;  // fcomp 0.0; test ah,0x40: unordered is "equal"
    return n;
}

double __cdecl Angle_Lerp(float a, float b, float t) {
    const X87Reg gap = std::fabs((X87Reg(a) - b).v);
    if (!(gap.v > kPi)) return (X87Reg(b) * t + (X87Reg(1.0f) - t) * a).v;
    const float around = (float)(X87Reg(kTwoPi) - gap);
    const X87Reg x = !(a >= 0.0f) ? (X87Reg(1.0f) - t) * around + b : X87Reg(around) * t + a;
    return (X87Reg(std::fmod((x - kMinusPiD).v, kTwoPiD)) - kPi).v;
}

double __cdecl Vec3_Dot(const float *a, const float *b) {
    return ((X87Reg(b[1]) * a[1] + X87Reg(b[2]) * a[2]) + X87Reg(a[0]) * b[0]).v;
}

void __cdecl Vec3_Normalize(float *v) {
    const X87Reg sum = (X87Reg(v[0]) * v[0] + X87Reg(v[1]) * v[1]) + X87Reg(v[2]) * v[2];
    const float length = (float)(sum.v != sum.v ? sum.v : X87_Round(std::sqrt(sum.v)));
    const float x = (float)(X87Reg(v[0]) / length), y = (float)(X87Reg(v[1]) / length);
    const float z = (float)(X87Reg(v[2]) / length);
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

void __cdecl Vec3_NormalizeXZ(float *v) {
    const X87Reg sum = X87Reg(v[0]) * v[0] + X87Reg(v[2]) * v[2];
    const float length = (float)(sum.v != sum.v ? sum.v : X87_Round(std::sqrt(sum.v)));
    const float x = (float)(X87Reg(v[0]) / length), y = (float)(X87Reg(v[1]) / length);
    const float z = (float)(X87Reg(v[2]) / length);
    v[0] = x;
    v[1] = y;
    v[2] = z;
}
}
