#include "crtpow.h"

#include "crtfloat.h"

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

}
