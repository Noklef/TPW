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
}
