// The game's x87 arithmetic where float arithmetic does not reproduce it. The FPU runs at
// 24-bit precision (see curve.h), but its registers keep their 15-bit exponent, and fild
// loads a 32-bit integer exactly: fild n; fmul f rounds the exact (up to 56-bit) product
// once, where (float)n * f would round n first. These return the register's value as a
// double, which holds every 24-bit significand exactly, whatever the exponent.
//
// Policy for reimplementations: write an original's float expressions with X87Reg, in its
// order of operations, storing to float only where it stores, so that results match bit for
// bit for every input, including overflow past float range and NaNs. Plain float or double
// is fine only where the bench (tools/fncmp.py, normal and --wild) shows no difference.
// Which of two NaNs comes out depends on operand order; keep the original's order where it
// costs nothing, but don't contort code for it (a commuted operand that differs only in
// NaN payloads is an accepted difference, noted in the function's header). A float moved
// with fld/fstp quiets a signalling NaN where a compiler's copy would not: X87_FloatThrough.
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

// fild n (a signed dword); fmul d (a double constant): the exact product of up to 32 and 53
// bits rounded once, where n * d in double would round to 53 bits first.
inline double X87_MulIntDouble(s32 n, double d) {
    if (n == 0 || d == 0 || !std::isfinite(d)) return (double)n * d;
    int e;
    const double frac = std::frexp(std::fabs(d), &e);
    const unsigned long long m = (unsigned long long)std::ldexp(frac, 53);
    const unsigned long long k = n < 0 ? 0ull - (unsigned long long)(long long)n : (unsigned long long)n;
    // k * m = top * 2^32 + rest (top below 2^54).
    const unsigned long long lo = k * (m & 0xffffffffull);
    const unsigned long long top = k * (m >> 32) + (lo >> 32), rest = lo & 0xffffffffull;
    double r;
    if (top < (1ull << 32)) {
        r = X87_Round24((top << 32) | rest, e - 53);
    } else {
        // At least 33 bits, so 9 or more go; rest only decides a tie, as a sticky bit.
        r = X87_Round24(top | (rest != 0 ? 1 : 0), e - 53 + 32);
    }
    return (n < 0) != (d < 0) ? -r : r;
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

// fpatan (y in st1, the destination; x in st0): atan2, except that NaNs come through as the
// x87 has them (two: the larger significand, y on a tie), where the CRT's may not.
inline double X87_Atan2(double y, double x) { return y != y || x != x ? X87_Add(y, x) : std::atan2(y, x); }

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
inline X87Reg operator/(X87Reg a, X87Reg b) {
    return a.v != a.v && b.v != b.v ? X87_Add(a.v, b.v) : X87_Round(a.v / b.v);
}
// fsubr / fdivr (and their p forms): src - dst and src / dst. Between two NaNs of equal
// significand src, the left operand, wins, as in x87emu (which the hardware's tie rule has
// not been checked against).
inline X87Reg X87_SubR(X87Reg dst, X87Reg src) { return src - dst; }
inline X87Reg X87_DivR(X87Reg dst, X87Reg src) { return src / dst; }

// fsin / fcos (unaffected by the precision control): infinity is invalid and gives the
// indefinite (negative quiet NaN), a NaN comes through, and from 2^63 on st0 is left as it is
// (C2 set; the game never checks it).
inline double X87_Trig(double x, double (*f)(double)) {
    if (x != x || std::fabs(x) >= 9223372036854775808.0) {
        if (!std::isinf(x)) return x;
        const unsigned long long indefinite = 0xfff8000000000000ull;
        std::memcpy(&x, &indefinite, 8);
        return x;
    }
    return f(x);
}
inline double X87_Sin(double x) { return X87_Trig(x, [](double v) { return std::sin(v); }); }
inline double X87_Cos(double x) { return X87_Trig(x, [](double v) { return std::cos(v); }); }
// fsqrt: rounded at the precision control; a NaN comes through, a negative is the indefinite.
inline X87Reg X87_Sqrt(X87Reg x) { return x.v != x.v ? x : X87Reg(X87_Round(std::sqrt(x.v))); }

// A float loaded and stored by the x87 (fld m32; fstp m32), where the compiler would copy it.
inline float X87_Through(float x) {
    u32 b;
    std::memcpy(&b, &x, 4);
    b = X87_FloatThrough(b);
    std::memcpy(&x, &b, 4);
    return x;
}
