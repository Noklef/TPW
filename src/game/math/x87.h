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
// Of two NaNs the x87 gives the one with the larger significand, the positive one of equal
// significands, whatever the operand order (SSE, which plain float code compiles to, gives
// the first operand); a signalling NaN memory operand gives way to a NaN in the register (an
// X87Reg's operation with a float: see X87Reg). A float moved with fld/fstp quiets a signalling NaN where a compiler's copy
// would not: X87_FloatThrough. (Measured on the hardware: tests/test_main.cpp's
// test_x87_nan_rule; tools/x87emu.py follows the same rule.)
#pragma once

#include <cmath>
#include <cstring>
#include <type_traits>

#include "../../../re/types.h"

// A double's bits, and back. (Every X87Reg operation rounds, so the rounding works on the bits:
// frexp, ldexp and 64-bit variable shifts, which a 32-bit build makes calls of, made a hooked
// game several times slower than the original.)
inline unsigned long long X87_Bits(double x) {
    unsigned long long b;
    std::memcpy(&b, &x, 8);
    return b;
}
inline double X87_FromBits(unsigned long long b) {
    double x;
    std::memcpy(&x, &b, 8);
    return x;
}

// m * 2^e rounded to a 24-bit significand, to nearest, ties to even.
inline double X87_Round24(unsigned long long m, int e) {
    if (m == 0) return 0.0;
    const u32 high = (u32)(m >> 32);
    int bits = high ? 32 : 0;
    for (u32 w = high ? high : (u32)m; w; w >>= 1) ++bits;
    if (bits > 24) {
        const int shift = bits - 24;
        const unsigned long long half = 1ull << (shift - 1), rest = m & ((half << 1) - 1);
        m >>= shift;
        if (rest > half || (rest == half && (m & 1))) ++m;
        e += shift;
    }
    // m is below 2^25 now: times a power of two, exact while the result is a normal double.
    if (e >= -1022 && e <= 1023 - 25) return (double)(u32)m * X87_FromBits((unsigned long long)(e + 1023) << 52);
    return std::ldexp((double)(u32)m, e);
}

// |x| (finite, not zero) as m * 2^(e - 53), m of 53 bits (the top one set): frexp's exponent and its fraction times 2^53.
inline unsigned long long X87_Significand(double x, int &e) {
    const unsigned long long b = X87_Bits(x);
    const int exponent = (int)(b >> 52) & 0x7ff;
    if (exponent == 0) return (unsigned long long)std::ldexp(std::frexp(std::fabs(x), &e), 53);
    e = exponent - 1022;
    return (b & 0xfffffffffffffull) | (1ull << 52);
}

// x (finite) rounded to a 24-bit significand: the x87 result of an operation whose exact
// value x is the correctly rounded double of. For + - * / of 24-bit operands that double
// rounding is harmless (53 >= 2 * 24 + 2).
inline double X87_Round(double x) {
    const unsigned long long b = X87_Bits(x);
    const u32 exponent = (u32)(b >> 52) & 0x7ff;
    if (exponent == 0x7ff) return x;
    if (exponent == 0) {   // a zero, or a denormal: rounded on its own leading bit
        if (x == 0) return x;
        int e;
        const unsigned long long m = X87_Significand(x, e);
        const double r = X87_Round24(m, e - 53);
        return x < 0 ? -r : r;
    }
    // A normal double: the low 29 of its 52 fraction bits go, to nearest, ties to even; a carry
    // goes on into the exponent (to infinity past the double range).
    return X87_FromBits((b + 0x0fffffff + ((b >> 29) & 1)) & ~0x1fffffffull);
}

// fild n (n as an unsigned 32-bit value: a qword with a zero high half); fmul f.
inline double X87_MulInt(u32 n, float f) {
    if (n == 0 || f == 0 || !std::isfinite(f)) return (double)n * (double)f;
    int e;
    const unsigned long long m = X87_Significand(f, e) >> 29;   // (exact: a float has 24 bits)
    const double r = X87_Round24((unsigned long long)n * m, e - 24);
    return f < 0 ? -r : r;
}

// fild n (a signed dword); fmul d (a double constant): the exact product of up to 32 and 53
// bits rounded once, where n * d in double would round to 53 bits first.
inline double X87_MulIntDouble(s32 n, double d) {
    if (n == 0 || d == 0 || !std::isfinite(d)) return (double)n * d;
    int e;
    const unsigned long long m = X87_Significand(d, e);
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

// fild qword (n, 0), n an unsigned dword; fmul d (a double): as X87_MulIntDouble.
inline double X87_MulUIntDouble(u32 n, double d) {
    if (n == 0 || d == 0 || !std::isfinite(d)) return (double)n * d;
    int e;
    const unsigned long long m = X87_Significand(d, e);
    const unsigned long long k = n;
    const unsigned long long lo = k * (m & 0xffffffffull);
    const unsigned long long top = k * (m >> 32) + (lo >> 32), rest = lo & 0xffffffffull;
    const double r = top < (1ull << 32) ? X87_Round24((top << 32) | rest, e - 53) : X87_Round24(top | (rest != 0 ? 1 : 0), e - 53 + 32);
    return d < 0 ? -r : r;
}

// fld f (a float); fmul d (a double): the exact product of up to 24 and 53 bits rounded once, as X87_MulIntDouble. A NaN, an infinity or a
// zero comes out as the double product has it.
inline double X87_MulFloatDouble(float f, double d) {
    const double x = f;
    if (x == 0 || d == 0 || !std::isfinite(x) || !std::isfinite(d)) return x * d;
    int ex, ed;
    const unsigned long long k = X87_Significand(x, ex) >> 29;
    const unsigned long long m = X87_Significand(d, ed);
    const unsigned long long lo = k * (m & 0xffffffffull);
    const unsigned long long top = k * (m >> 32) + (lo >> 32), rest = lo & 0xffffffffull;
    const int e = ex - 24 + ed - 53;
    const double r = top < (1ull << 32) ? X87_Round24((top << 32) | rest, e) : X87_Round24(top | (rest != 0 ? 1 : 0), e + 32);
    return (x < 0) != (d < 0) ? -r : r;
}

// a + b (doubles, e.g. a fild'd int and a float) rounded once to a 24-bit significand, as
// fild n; fadd f does. The double sum can round onto a 24-bit midpoint and then tie the
// wrong way; its exact error (TwoSum) says which side of the midpoint the true sum is on.
// A NaN operand comes through as the x87 has it: both NaNs, the one with the larger
// significand (the positive one of equal significands), where SSE would take the first operand.
inline double X87_NanPick(double a, double b) {
    unsigned long long ma, mb;
    std::memcpy(&ma, &a, 8);
    std::memcpy(&mb, &b, 8);
    const unsigned long long sa = ma & ((1ull << 52) - 1), sb = mb & ((1ull << 52) - 1);
    if (sa != sb) return sa > sb ? a : b;
    return (ma >> 63) == 0 ? a : b;
}
inline double X87_Add(double a, double b) {
    if (a != a || b != b) {
        if (a != a && b != b) return X87_NanPick(a, b);
        return a != a ? a : b;
    }
    const double s = a + b;
    const unsigned long long sb = X87_Bits(s);
    const u32 exponent = (u32)(sb >> 52) & 0x7ff;
    if (exponent == 0x7ff || s == 0) return s;
    const double bb = s - a, e = (a - (s - bb)) + (b - bb);
    if (e == 0) return X87_Round(s);
    if (exponent == 0) {   // a denormal sum: its significand taken from its own leading bit
        int exp;
        const unsigned long long m = X87_Significand(s, exp);
        if ((m & ((1ull << 29) - 1)) != (1ull << 28)) return X87_Round(s);
        const unsigned long long kept = (m >> 29) + (((e > 0) == (s > 0)) ? 1 : 0);
        const double r = std::ldexp((double)kept, exp - 24);
        return s < 0 ? -r : r;
    }
    if (((u32)sb & 0x1fffffff) != 0x10000000) return X87_Round(s);
    // On a midpoint: down to the 24 bits, then up one if the exact sum is past it (away from zero).
    return X87_FromBits((sb & ~0x1fffffffull) + ((e > 0) == (s > 0) ? 0x20000000 : 0));
}

// fld a; fmul b (both doubles): the exact product of up to 53 by 53 bits rounded once to 24 bits. The double product and its exact
// error (fma) place it as X87_Add places a sum. Exact for normal double products; a NaN, an infinity, a zero, and products past the
// double range or in its denormals come out as the double product has them (the x87's register would keep its 15-bit exponent).
inline double X87_MulDouble(double a, double b) {
    const double p = a * b;
    if (!std::isfinite(p) || p == 0 || std::fabs(p) < 0x1p-969) return p;
    return X87_Add(p, std::fma(a, b, -p));
}

// fpatan (y in st1, the destination; x in st0): atan2, except that NaNs come through as the
// x87 has them (two: X87_NanPick), where the CRT's may not.
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
// the result is X87_NanPick's, whatever the order. At least one operand of each operation
// has to be an X87Reg, or C++ does it in float. A float operand (not an X87Reg) stands for a
// memory operand (fadd m32 and the like): a signalling NaN there gives way to a NaN X87Reg as
// it is, with no significands compared; X87Reg(f) is f loaded with fld (quiet) first, for a
// register form (faddp and the like).
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
// fsubr / fdivr (and their p forms): src - dst and src / dst.
inline X87Reg X87_SubR(X87Reg dst, X87Reg src) { return src - dst; }
inline X87Reg X87_DivR(X87Reg dst, X87Reg src) { return src / dst; }

// An X87Reg's + - * / with a float memory operand, either side (fadd / fsub / fsubr / fmul /
// fdiv / fdivr m32): a signalling NaN m gives way to a NaN st as it is; otherwise the register
// operation on m (quiet, as the conversion to double makes it). (Templates, so that only a
// float takes them: an int or a double converts to an X87Reg as before.)
inline bool X87_MemGivesWay(X87Reg st, float m) {
    u32 b;
    std::memcpy(&b, &m, 4);
    return st.v != st.v && (b & 0x7f800000) == 0x7f800000 && (b & 0x7fffff) && !(b & 0x400000);
}
template <class F, class = std::enable_if_t<std::is_same<F, float>::value>> inline X87Reg operator+(X87Reg a, F m) {
    return X87_MemGivesWay(a, m) ? a : a + X87Reg(m);
}
template <class F, class = std::enable_if_t<std::is_same<F, float>::value>> inline X87Reg operator+(F m, X87Reg a) {
    return X87_MemGivesWay(a, m) ? a : X87Reg(m) + a;
}
template <class F, class = std::enable_if_t<std::is_same<F, float>::value>> inline X87Reg operator-(X87Reg a, F m) {
    return X87_MemGivesWay(a, m) ? a : a - X87Reg(m);
}
template <class F, class = std::enable_if_t<std::is_same<F, float>::value>> inline X87Reg operator-(F m, X87Reg a) {
    return X87_MemGivesWay(a, m) ? a : X87Reg(m) - a;
}
template <class F, class = std::enable_if_t<std::is_same<F, float>::value>> inline X87Reg operator*(X87Reg a, F m) {
    return X87_MemGivesWay(a, m) ? a : a * X87Reg(m);
}
template <class F, class = std::enable_if_t<std::is_same<F, float>::value>> inline X87Reg operator*(F m, X87Reg a) {
    return X87_MemGivesWay(a, m) ? a : X87Reg(m) * a;
}
template <class F, class = std::enable_if_t<std::is_same<F, float>::value>> inline X87Reg operator/(X87Reg a, F m) {
    return X87_MemGivesWay(a, m) ? a : a / X87Reg(m);
}
template <class F, class = std::enable_if_t<std::is_same<F, float>::value>> inline X87Reg operator/(F m, X87Reg a) {
    return X87_MemGivesWay(a, m) ? a : X87Reg(m) / a;
}

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
