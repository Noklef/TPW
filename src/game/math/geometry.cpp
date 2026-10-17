#include "geometry.h"

#include <cstdint>

#include "x87.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

double __cdecl Vec3_DotByValue(float ax, float ay, float az, float bx, float by, float bz) {
    return (X87Reg(bx) * ax + X87Reg(by) * ay + X87Reg(bz) * az).v;
}

u32 __cdecl Tri_WindingTest(const float *a, const float *b, const float *c) {
    const u32 *pa = reinterpret_cast<const u32 *>(a), *pb = reinterpret_cast<const u32 *>(b), *pc = reinterpret_cast<const u32 *>(c);
    global<u32>(0x008c63bc) = pa[0];
    global<u32>(0x008c63c8) = pa[1];
    global<u32>(0x008c63b0) = pa[3];
    global<u32>(0x008c63c0) = pb[0];
    global<u32>(0x008c63cc) = pb[1];
    global<u32>(0x008c63b4) = pb[3];
    global<u32>(0x008c63c4) = pc[0];
    global<u32>(0x008c63d0) = pc[1];
    global<u32>(0x008c63b8) = pc[3];
    const float xa = global<float>(0x008c63bc), ya = global<float>(0x008c63c8), wa = global<float>(0x008c63b0);
    const float xb = global<float>(0x008c63c0), yb = global<float>(0x008c63cc), wb = global<float>(0x008c63b4);
    const float xc = global<float>(0x008c63c4), yc = global<float>(0x008c63d0), wc = global<float>(0x008c63b8);
    const X87Reg f = (X87Reg(wa) * yb - X87Reg(wb) * ya) * xc - X87Reg(wc) * yb * xa;
    const X87Reg e = (X87Reg(wa) * yc - X87Reg(wc) * ya) * xb - X87Reg(wb) * yc * xa;
    return f.v < e.v || f.v != f.v || e.v != e.v ? 1 : 0;
}
}
