#include "quad.h"

#include <cmath>
#include <cstdint>

#include "../math/x87.h"

namespace {

// a * b rounded once to a 24-bit significand, for a full-precision a (fsin / fcos's result) where the double product would round
// to 53 bits first: only a double landing on a 24-bit midpoint can round the wrong way, and the product's exact error says which.
// a_first: a is the x87 destination (which of two NaNs comes through).
X87Reg MulRound(double a, X87Reg b, bool a_first) {
    if (a != a || b.v != b.v) return a_first ? X87Reg(a) * b : b * X87Reg(a);
    const double p = a * b.v;
    if (!std::isfinite(p) || p == 0) return p;
    int e;
    const double frac = std::frexp(std::fabs(p), &e);
    const unsigned long long m = (unsigned long long)std::ldexp(frac, 53);
    if ((m & ((1ull << 29) - 1)) != (1ull << 28)) return X87_Round(p);
    const double err = std::fma(a, b.v, -p);
    if (err == 0) return X87_Round(p);
    const double r = std::ldexp((double)((m >> 29) + ((err > 0) == (p > 0) ? 1 : 0)), e - 24);
    return p < 0 ? -r : r;
}

s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }

// One vertex's coordinates a and b turned about (ca, cb), each first stored as its offset from the centre.
void Turn(float &a, float &b, double sin, double cos, X87Reg ca, X87Reg cb) {
    a = (float)(X87Reg(a) - ca);
    const X87Reg db = X87Reg(b) - cb;
    const X87Reg sdb = MulRound(sin, db, true);
    b = (float)db;
    const float da = a;
    a = (float)((sdb + MulRound(cos, da, false)) + ca);
    b = (float)((MulRound(cos, b, true) - MulRound(sin, da, false)) + cb);
}

}  // namespace

extern "C" {

void __cdecl Quad_TurnXY(float *q, float angle, const float *centre) {
    X87Reg cx = 0.0, cy = 0.0;
    if (centre) {
        cx = centre[0];
        cy = centre[1];
    } else {
        cx = (X87Reg(q[4]) + q[0]) * 0.5f;  // 0x7025c4
        cy = (X87Reg(q[1]) + q[9]) * 0.5f;
    }
    const float *sines = reinterpret_cast<const float *>(0x008c63d8);
    const float cos = sines[(u32)(0x40 - Ftol(X87Reg(angle) * -256.0f)) & 0xff];  // 0x7025c8
    const float sin = sines[(u32)Ftol(X87Reg(angle) * 256.0f) & 0xff];            // 0x7025cc
    for (int i = 12; i >= 0; i -= 4) Turn(q[i], q[i + 1], sin, cos, cx, cy);
}

void __cdecl Quad_TurnXZ(float *q, float angle) {
    const double cos = X87_Cos(angle), sin = X87_Sin(angle);
    const X87Reg cx = (X87Reg(q[4]) + q[0]) * 0.5f;
    const float cz = (float)((X87Reg(q[10]) + q[2]) * 0.5f);
    for (int i = 12; i >= 0; i -= 4) Turn(q[i], q[i + 2], sin, cos, cx, cz);
}
}
