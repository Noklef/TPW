#include "crtpow.h"

#include "crtfloat.h"
#include "crtfpexcept.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 low(double v) {
    u32 w[2];
    std::memcpy(w, &v, 8);
    return w[0];
}

u32 high(double v) {
    u32 w[2];
    std::memcpy(w, &v, 8);
    return w[1];
}

// FCOM's condition codes as the listing tests them: C0 (below, or unordered), C0 | C3 (not above), C3 (equal, or unordered).
bool below_or_unordered(double a, double b) { return !(a >= b); }
bool not_above(double a, double b) { return !(a > b); }
bool equal_or_unordered(double a, double b) { return !(a < b) && !(a > b); }

void put(double *to, u32 lo, u32 hi) {
    u32 *const w = reinterpret_cast<u32 *>(to);
    w[0] = lo;
    w[1] = hi;
}

// The global double's two dwords, read live (writable data).
void put_global(double *to, u32 address) { put(to, global<u32>(address), global<u32>(address + 4)); }

// _CIpow's values as the x87 holds them (80 bits), exact between the asm blocks.
struct X80 {
    u8 b[10];
};

X80 x80_global(u32 address) {
    X80 r;
    std::memcpy(r.b, reinterpret_cast<const void *>((uintptr_t)address), 10);
    return r;
}

X80 x80_double(double v) {
    X80 r;
    X80 *const p = &r;
    __asm {
        fld v
        mov eax, p
        fstp tbyte ptr [eax]
    }
    return r;
}

// _fload 0x6861b5: a double onto the x87; an infinity or NaN made by hand (a signalling NaN stays one).
X80 fload(const u32 *w) {
    if ((w[1] & 0x7ff00000) != 0x7ff00000) {
        double v;
        std::memcpy(&v, w, 8);
        return x80_double(v);
    }
    X80 r;
    const uint64_t mantissa = ((uint64_t)w[1] << 32 | w[0]) << 11;
    const u16 top = (u16)((w[1] | 0x7fff0000) >> 16);
    std::memcpy(r.b, &mantissa, 8);
    std::memcpy(r.b + 8, &top, 2);
    return r;
}

// FADDP of y (ST1) and x (ST0): a NaN's way through.
X80 add(const X80 &y, const X80 &x) {
    X80 r;
    const X80 *py = &y, *px = &x, *pr = &r;
    __asm {
        mov eax, py
        fld tbyte ptr [eax]
        mov eax, px
        fld tbyte ptr [eax]
        faddp st(1), st
        mov eax, pr
        fstp tbyte ptr [eax]
    }
    return r;
}

// _isintTOS 0x67c7c5: 0 for a y that is not an integer, 1 odd, 2 even (the half by the 0.5 at 0x77e7b0, writable data).
u32 isint(const X80 &y) {
    const X80 *py = &y;
    u32 kind;
    __asm {
        xor ecx, ecx
        mov eax, py
        fld tbyte ptr [eax]
        fld st(0)
        frndint
        fcomp st(1)
        fnstsw ax
        sahf
        jnz not_integer
        fmul qword ptr ds:[0x0077e7b0]
        inc ecx
        fld st(0)
        frndint
        fcompp
        fnstsw ax
        sahf
        jnz done
        inc ecx
        jmp done
    not_integer:
        fstp st(0)
    done:
        mov kind, ecx
    }
    return kind;
}

// FYL2X then _ffexpm1 0x686170: 2^(y log2 x) at the precision control.
X80 power(const X80 &y, const X80 &x) {
    X80 r;
    const X80 *py = &y, *px = &x, *pr = &r;
    __asm {
        mov eax, py
        fld tbyte ptr [eax]
        mov eax, px
        fld tbyte ptr [eax]
        fyl2x
        fld st(0)
        frndint
        fsubr st(1), st
        fxch st(1)
        fchs
        f2xm1
        fld1
        faddp st(1), st
        fscale
        fstp st(1)
        mov eax, pr
        fstp tbyte ptr [eax]
    }
    return r;
}

void fchs(X80 &v) { v.b[9] ^= 0x80; }

void restore(u16 saved) {
    if (saved != 0x27f) __asm fldcw saved
}

// _check_overflow_exit 0x686259 (pow's): a double's underflow (r scaled by 2^1536, flushed below DBL_MIN) or overflow (by 2^-1536,
// infinite past DBL_MAX) is an error of type 4 or 3, an inexact result one of type 8 when the caller unmasked precision; else 0 and
// the caller's control word back.
u32 check_overflow(X80 &r, u16 saved) {
    X80 *const pr = &r;
    double d;
    __asm {
        mov eax, pr
        fld tbyte ptr [eax]
        fstp d
    }
    const u32 exponent = high(d) & 0x7ff00000;
    if (exponent == 0) {
        __asm {
            fld qword ptr ds:[0x00708d2c]
            mov eax, pr
            fld tbyte ptr [eax]
            fscale
            fstp st(1)
            fld st(0)
            fabs
            fcomp qword ptr ds:[0x00708d1c]
            fnstsw ax
            sahf
            jnc kept
            fmul qword ptr ds:[0x00708d3c]
        kept:
            mov eax, pr
            fstp tbyte ptr [eax]
        }
        return 4;
    }
    if (exponent == 0x7ff00000) {
        __asm {
            fld qword ptr ds:[0x00708d24]
            mov eax, pr
            fld tbyte ptr [eax]
            fscale
            fstp st(1)
            fld st(0)
            fabs
            fcomp qword ptr ds:[0x00708d14]
            fnstsw ax
            sahf
            jbe kept2
            fmul qword ptr ds:[0x00708d34]
        kept2:
            mov eax, pr
            fstp tbyte ptr [eax]
        }
        return 3;
    }
    if (saved == 0x27f) return 0;
    if (!(saved & 0x20)) {
        u16 status;
        __asm fstsw status
        if (status & 0x20) return 8;
    }
    __asm fldcw saved
    return 0;
}

// _startTwoArgErrorHandling 0x686110 (pow's): the error record (type, "pow" 0x77e7b8, x, y, r as a double) through _87except
// (opcode 0x1d), r its result back, then the caller's control word.
void error(u32 type, X80 &r, const u32 *x, const u32 *y, u16 saved) {
    u32 exc[8] = {type, 0x0077e7b8, x[0], x[1], y[0], y[1]};
    u32 *const retval = exc + 6;
    X80 *const pr = &r;
    __asm {
        mov eax, pr
        fld tbyte ptr [eax]
        mov eax, retval
        fstp qword ptr [eax]
    }
    u16 cw = saved;
    CRT_87except(0x1d, reinterpret_cast<s32 *>(exc), &cw);
    double result;
    std::memcpy(&result, retval, 8);
    r = x80_double(result);
    restore(saved);
}

}  // namespace

extern "C" {

double __cdecl CRT_frnd(double x) {
    double r;
    __asm {
        fld x
        frndint
        fstp r
    }
    return r;
}

s32 __cdecl CRT_d_inttype(double y) {
    if (CRT_fpclass(low(y), high(y)) & 0x90) return 0;
    if (!equal_or_unordered(CRT_frnd(y), y)) return 0;
    static const double kTwo = 2.0;
    double half;
    __asm {   // (on the x87 at its precision, as the original)
        fld y
        fdiv kTwo
        fstp half
    }
    return equal_or_unordered(CRT_frnd(half), half) ? 2 : 1;
}

s32 __cdecl CRT_powhlp(double x, double y, double *result) {
    const double ax = below_or_unordered(x, 0.0) ? -x : x;
    const u32 ylo = low(y), yhi = high(y);
    if (yhi == 0x7ff00000 && ylo == 0) {
        if (!not_above(ax, 1.0)) return put_global(result, 0x00781198), 0;
        if (below_or_unordered(ax, 1.0)) return put(result, 0, 0), 0;
        return put_global(result, 0x007811a0), 1;
    }
    if (yhi == 0xfff00000 && ylo == 0) {
        if (!not_above(ax, 1.0)) return put(result, 0, 0), 0;
        if (!below_or_unordered(ax, 1.0)) return put_global(result, 0x007811a0), 1;
        return put_global(result, 0x00781198), 0;
    }
    const u32 xlo = low(x), xhi = high(x);
    if (xhi == 0x7ff00000 && xlo == 0) {
        if (!not_above(y, 0.0)) return put_global(result, 0x00781198), 0;
        return put(result, 0, below_or_unordered(y, 0.0) ? 0 : 0x3ff00000), 0;
    }
    if (xhi == 0xfff00000 && xlo == 0) {
        const s32 type = CRT_d_inttype(y);
        if (!not_above(y, 0.0)) {
            if (type != 1) return put_global(result, 0x00781198), 0;
            double negated;
            __asm {   // (through the x87: a signalling NaN there loads quiet)
                fld qword ptr ds:[0x00781198]
                fchs
                fstp negated
            }
            *result = negated;
            return 0;
        }
        if (below_or_unordered(y, 0.0)) return type == 1 ? put_global(result, 0x007811b8) : put(result, 0, 0), 0;
        return put(result, 0, 0x3ff00000), 0;
    }
    return 0;
}

void __cdecl CRT_CIpow(double x, double y, u8 *io) {
    const u32 *const xw = reinterpret_cast<const u32 *>(&x);
    const u32 *const yw = reinterpret_cast<const u32 *>(&y);
    X80 y80;
    std::memcpy(y80.b, io, 10);
    u16 saved;
    __asm fstcw saved
    if (saved != 0x27f) {   // (_load_CW 0x686185: the caller's precision, rounding to nearest, all masked)
        const u16 masked = (saved & 0x300) | 0x7f;
        __asm fldcw masked
    }
    const bool fast = CRT_fastflag != 0;   // (__fastflag: no error handling)
    X80 x80 = fload(xw);
    X80 r;
    u32 type = 0;
    const bool x_special = (xw[1] & 0x7ff00000) == 0x7ff00000, y_special = (yw[1] & 0x7ff00000) == 0x7ff00000;
    const bool x_nan = x_special && ((xw[1] & 0xfffff) | xw[0]), y_nan = y_special && ((yw[1] & 0xfffff) | yw[0]);
    if (y_nan && (yw[1] & 0x80000)) {
        r = add(y80, x80);
        type = 1;
    } else if (x_special || y_special) {
        if (x_nan) {
            r = add(y80, x80);
            type = (xw[1] & 0x80000) ? 1 : 7;
        } else if (y_nan) {
            r = add(y80, x80);
            type = 7;
        } else {   // (an infinity: _powhlp, the caller's other registers saved round it)
            double xd, yd, result;
            const X80 *px = &x80, *py = &y80;
            __asm {
                mov eax, px
                fld tbyte ptr [eax]
                fstp xd
                mov eax, py
                fld tbyte ptr [eax]
                fstp yd
            }
            u8 env[108];
            __asm fsave env
            const s32 domain = CRT_powhlp(xd, yd, &result);
            __asm frstor env
            r = x80_double(result);
            type = domain ? 1 : 0;
        }
    } else if (!(xw[1] & 0x7ff00000) && !((xw[1] & 0xfffff) | xw[0])) {   // x +-0
        if (!(yw[1] & 0x7ff00000)) {
            r = x80_double(((yw[1] & 0xfffff) | yw[0]) ? 0.0 : 1.0);
        } else {
            const u32 odd_negative = (xw[1] >> 31) & isint(y80);
            if (yw[1] & 0x80000000) {
                r = x80_global(0x00781350);
                type = 2;
            } else {
                r = x80_double(0.0);
            }
            if (odd_negative) fchs(r);
        }
    } else {
        const bool negative = (xw[1] & 0x80000000) != 0;
        u32 kind = 0;
        if (negative) {
            kind = isint(y80);
            fchs(x80);
        }
        if (negative && !kind) {
            r = x80_global(0x00780f30);
            type = 1;
        } else {
            r = power(y80, x80);
            if (kind == 1) fchs(r);
            if (fast) {
                restore(saved);
            } else {
                type = check_overflow(r, saved);
                if (type) error(type, r, xw, yw, saved);
            }
            std::memcpy(io, r.b, 10);
            return;
        }
    }
    if (type && !fast) error(type, r, xw, yw, saved);
    else restore(saved);
    std::memcpy(io, r.b, 10);
}

}
