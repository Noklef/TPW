// The game's x87 arithmetic where float arithmetic does not reproduce it. The FPU runs at
// 24-bit precision (see curve.h), but its registers keep their 15-bit exponent, and fild
// loads a 32-bit integer exactly: fild n; fmul f rounds the exact (up to 56-bit) product
// once, where (float)n * f would round n first. These return the register's value as a
// double, which holds every 24-bit significand exactly, whatever the exponent.
#pragma once

#include <cmath>

#include "../../../re/types.h"

// m * 2^e rounded to a 24-bit significand, to nearest, ties to even.
inline double X87_Round24(unsigned long long m, int e) {
    if (m == 0) return 0.0;
    int bits = 0;
    while (bits < 64 && (m >> bits) != 0) ++bits;
    if (bits > 24) {
        const int shift = bits - 24;
        const unsigned long long half = 1ull << (shift - 1), rest = m & ((half << 1) - 1);
        m >>= shift;
        if (rest > half || (rest == half && (m & 1))) ++m;
        e += shift;
    }
    return std::ldexp((double)m, e);
}

// x (finite) rounded to a 24-bit significand: the x87 result of an operation whose exact
// value x is the correctly rounded double of. For + - * / of 24-bit operands that double
// rounding is harmless (53 >= 2 * 24 + 2).
inline double X87_Round(double x) {
    if (x == 0 || !std::isfinite(x)) return x;
    int e;
    const double frac = std::frexp(std::fabs(x), &e);
    const double r = X87_Round24((unsigned long long)std::ldexp(frac, 53), e - 53);
    return x < 0 ? -r : r;
}

// fild n (n as an unsigned 32-bit value: a qword with a zero high half); fmul f.
inline double X87_MulInt(u32 n, float f) {
    if (n == 0 || f == 0 || !std::isfinite(f)) return (double)n * (double)f;
    int e;
    const double frac = std::frexp(std::fabs((double)f), &e);
    const double r = X87_Round24((unsigned long long)n * (unsigned long long)std::ldexp(frac, 24), e - 24);
    return f < 0 ? -r : r;
}

// A float loaded and stored by the x87 (fld m32; fstp m32), as bits: the value is kept,
// except that a signalling NaN comes out quiet.
inline u32 X87_FloatThrough(u32 bits) {
    if ((bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff) && !(bits & 0x400000)) bits |= 0x400000;
    return bits;
}
