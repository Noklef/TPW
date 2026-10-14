// The game's x87 arithmetic where float arithmetic does not reproduce it. The FPU runs at
// 24-bit precision (see curve.h), but its registers keep their 15-bit exponent, and fild
// loads a 32-bit integer exactly: fild n; fmul f rounds the exact (up to 56-bit) product
// once, where (float)n * f would round n first. These return the register's value as a
// double, which holds every 24-bit significand exactly, whatever the exponent.
#pragma once

#include <cmath>
#include <cstring>

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

// a + b (doubles, e.g. a fild'd int and a float) rounded once to a 24-bit significand, as
// fild n; fadd f does. The double sum can round onto a 24-bit midpoint and then tie the
// wrong way; its exact error (TwoSum) says which side of the midpoint the true sum is on.
// A NaN operand comes through as the x87 has it: both NaNs, the one with the larger
// significand (a, the destination, on a tie), where SSE would take the first operand.
inline double X87_Add(double a, double b) {
    if (a != a || b != b) {
        if (a != a && b != b) {
            unsigned long long ma, mb;
            std::memcpy(&ma, &a, 8);
            std::memcpy(&mb, &b, 8);
            return (ma & ((1ull << 52) - 1)) >= (mb & ((1ull << 52) - 1)) ? a : b;
        }
        return a != a ? a : b;
    }
    const double s = a + b;
    if (!std::isfinite(s) || s == 0) return s;
    const double bb = s - a, e = (a - (s - bb)) + (b - bb);
    int exp;
    const double frac = std::frexp(std::fabs(s), &exp);
    const unsigned long long m = (unsigned long long)std::ldexp(frac, 53);
    if (e == 0 || (m & ((1ull << 29) - 1)) != (1ull << 28)) return X87_Round(s);
    const unsigned long long kept = (m >> 29) + (((e > 0) == (s > 0)) ? 1 : 0);
    const double r = std::ldexp((double)kept, exp - 24);
    return s < 0 ? -r : r;
}

// A float loaded and stored by the x87 (fld m32; fstp m32), as bits: the value is kept,
// except that a signalling NaN comes out quiet.
inline u32 X87_FloatThrough(u32 bits) {
    if ((bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff) && !(bits & 0x400000)) bits |= 0x400000;
    return bits;
}

// An x87 register at the game's precision, for writing an original's expression as it
// stands: + - * round the exact result once to a 24-bit significand, keeping the register's
// wide exponent (a float rounds again, or overflows, only when stored). Of two NaN operands
// the result is the one with the larger significand, the left operand (the destination) on
// a tie, so keep each operation's operands in the original's order. At least one operand
// of each operation has to be an X87Reg, or C++ does it in float.
struct X87Reg {
    double v;
    X87Reg(double x) : v(x) {}
    explicit operator float() const { return (float)v; }
};
inline X87Reg operator+(X87Reg a, X87Reg b) { return X87_Add(a.v, b.v); }
inline X87Reg operator-(X87Reg a, X87Reg b) {
    return a.v != a.v || b.v != b.v ? X87_Add(a.v, b.v) : X87_Add(a.v, -b.v);
}
inline X87Reg operator*(X87Reg a, X87Reg b) {
    return a.v != a.v && b.v != b.v ? X87_Add(a.v, b.v) : X87_Round(a.v * b.v);
}
