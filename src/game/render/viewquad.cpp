#include "viewquad.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../math/x87.h"

namespace {

// The bits of 1 / pi after the point.
const u32 kInvPi[6] = {0x517cc1b7, 0x27220a94, 0xfe13abe8, 0xfa9a6ee0, 0x6db14acc, 0x9e21c820};

// (Integers to doubles by signed conversions and powers of two by constants: the CRT's unsigned conversion and ldexp go through the
// x87, which runs at the game's 24 bits here.)
// A dword as a double.
double whole(u32 v) { return static_cast<double>(static_cast<s32>(v >> 1)) * 2.0 + static_cast<double>(static_cast<s32>(v & 1)); }

// fptan as the reference has it (in double, as fsin): exactly reduced. The CRT's x86 tan loses its reduction far out, so from 2^24 (an
// angle of the x87's precision is an integer m 2^k there, m below 2^24, k from 1 to 39 below 2^63) frac(|x| / pi) comes from the 96 bits
// of 1 / pi past the k-th, times m, and the tangent is that of pi times it.
double tangent_of(double x) {
    if (!(std::fabs(x) >= 16777216.0) || std::fabs(x) >= 9223372036854775808.0) return std::tan(x);
    uint64_t bits;
    std::memcpy(&bits, &x, 8);
    const u32 m = static_cast<u32>(bits >> 29) & 0x7fffff | 0x800000;
    const int skip = static_cast<int>(bits >> 52 & 0x7ff) - 1023 - 23;
    u32 w[3];
    for (int i = 0; i < 3; i++) {
        const int bit = skip + 32 * i, word = bit >> 5, shift = bit & 31;
        w[i] = shift != 0 ? kInvPi[word] << shift | kInvPi[word + 1] >> (32 - shift) : kInvPi[word];
    }
    const uint64_t p2 = static_cast<uint64_t>(m) * w[2];
    const uint64_t p1 = static_cast<uint64_t>(m) * w[1] + (p2 >> 32);
    const uint64_t p0 = static_cast<uint64_t>(m) * w[0] + (p1 >> 32);
    const double frac = whole(static_cast<u32>(p2)) * 0x1p-96 + whole(static_cast<u32>(p1)) * 0x1p-64 + whole(static_cast<u32>(p0)) * 0x1p-32;
    const double t = std::tan(frac * 3.14159265358979311600);
    return x < 0 ? -t : t;
}

}  // namespace

extern "C" {

void __fastcall ViewQuad_SetFov(float *quad, void *, float fov) {
    const X87Reg angle = X87Reg(fov) * 3.14159274f * 0.00277777785f;
    const X87Reg one = X87Reg(0.0f) - -1.0f;
    // fptan: from 2^63 on it pushes nothing, so the fxch after it meets an empty register (the indefinite) and the indefinite is what
    // comes out as the tangent (sic).
    double tangent = X87_Trig(angle.v, tangent_of);
    if (std::isfinite(angle.v) && std::fabs(angle.v) >= 9223372036854775808.0) {
        const unsigned long long indefinite = 0xfff8000000000000ull;
        std::memcpy(&tangent, &indefinite, 8);
    }
    const X87Reg h = X87Reg(tangent) * one * 0.5f;
    const float hf = static_cast<float>(h);
    const X87Reg w = h / *reinterpret_cast<const float *>((uintptr_t)0x008bcbcc);
    const float wf = static_cast<float>(w);
    quad[2] = quad[5] = quad[8] = quad[11] = 1.0f;
    quad[0] = -wf;
    quad[1] = hf;
    quad[3] = wf;
    quad[4] = hf;
    quad[6] = wf;
    quad[7] = -hf;
    quad[9] = -wf;
    quad[10] = -hf;
}
}
