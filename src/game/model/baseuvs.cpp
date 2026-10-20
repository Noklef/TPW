#include "baseuvs.h"

#include <cmath>
#include <cstdint>

#include "../core/globalmem.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// Texture coordinate i of the part: u, and v 0x10 bytes on (four to a 0x20-byte block).
float &u_of(u8 *part, u32 i) { return at<float>(at<u8 *>(part, 0x68), (i >> 2) * 0x20 + (i & 3) * 4); }
float &v_of(u8 *part, u32 i) { return at<float>(at<u8 *>(part, 0x68), (i >> 2) * 0x20 + (i & 3) * 4 + 0x10); }

// A coordinate snapped onto a whole number when its fraction is within the tolerance of one (the x87's compares: a NaN is below).
float snapped(float x, float tolerance, float high, float low) {
    double whole;
    const float fraction = (float)std::modf((double)x, &whole);   // CRT modf (0x67b280)
    const float w = (float)whole;
    X87Reg c = fraction;
    if (!(fraction >= low))
        c = -1.0f;
    else if (fraction > -tolerance && !(fraction >= tolerance))
        c = 0.0f;
    else if (fraction > high)
        c = 1.0f;
    return (float)(c + X87Reg(w));
}

// The floor of a triangle's least coordinate: its whole part, one less when the fraction is below 0 (or NaN).
float floor_of(float x) {
    double whole;
    const double fraction = std::modf((double)x, &whole);   // CRT modf (0x67b280)
    float w = (float)whole;
    if (!(fraction >= 0.0))
        w = (float)(X87Reg(w) - X87Reg(1.0f));
    return w;
}

bool outside(float u, float v) { return !(u >= 0.0f) || u > 1.0f || !(v >= 0.0f) || v > 1.0f; }

s32 ftol(X87Reg x) { return (s32)(long long)x.v; }   // __ftol (0x67a830)
}

extern "C" void __cdecl RideBase_TidyUVs(const u8 *, u8 *part, float tolerance, const u8 *map, const float *positions, u32 *warned) {
    if (at<u16>(part, 0x5e)) {
        const X87Reg rest = X87Reg(1.0f) - X87Reg(tolerance);
        const float high = (float)rest, low = (float)X87Reg(-rest.v);
        for (u32 i = 0; i < at<u16>(part, 0x5e); ++i) {
            const float v = v_of(part, i);
            const float u = snapped(u_of(part, i), tolerance, high, low);
            const float w = snapped(v, tolerance, high, low);
            u_of(part, i) = u;
            v_of(part, i) = w;
        }
    }
    u32 *const done = static_cast<u32 *>(GlobalMem_Alloc((u32)at<u16>(part, 0x5e) * 4, 1));
    float *const leastU = static_cast<float *>(GlobalMem_Alloc((u32)at<u16>(part, 0x5c) * 4, 1));
    float *const leastV = static_cast<float *>(GlobalMem_Alloc((u32)at<u16>(part, 0x5c) * 4, 1));
    for (u32 k = 0; k < at<u16>(part, 0x5c); ++k) {
        const u16 *const corner = &at<u16>(at<u8 *>(part, 0x70), 8 * k + 2);
        float lu = 3.40282347e+38f, lv = 3.40282347e+38f;   // (0x6fe77c)
        for (u32 j = 0; j < 3; ++j) {
            const float u = u_of(part, corner[j]), v = X87_Through(v_of(part, corner[j]));
            if (!(u >= lu))
                lu = X87_Through(u);
            if (!(v >= lv))
                lv = v;
        }
        leastU[k] = X87_Through(lu);
        leastV[k] = lv;
    }
    for (u32 k = 0; k < at<u16>(part, 0x5c); ++k) {
        const float wu = floor_of(leastU[k]), wv = floor_of(leastV[k]);
        const u16 *const corner = &at<u16>(at<u8 *>(part, 0x70), 8 * k + 2);
        for (u32 j = 0; j < 3; ++j) {
            if (done[corner[j]])
                continue;
            const float u = (float)(X87Reg(u_of(part, corner[j])) - X87Reg(wu));
            const float v = (float)(X87Reg(v_of(part, corner[j])) - X87Reg(wv));
            u_of(part, corner[j]) = u;
            v_of(part, corner[j]) = v;
            done[corner[j]] = 1;
        }
    }
    u8 *group = at<u8 *>(part, 0x6c);
    for (u32 g = 0; g < at<u16>(part, 0x5a); ++g, group += 0x10) {
        const u16 *corner = &at<u16>(at<u8 *>(part, 0x70), 8 * at<u16>(group, 4) + 2);
        u32 off = 0;
        float cx = 0, cy = 0, cz = 0;
        for (u32 k = 0; k < at<u16>(group, 6); ++k, corner += 4) {
            if (map && positions) {
                const u16 *const index = at<u16 *>(part, 0x94);
                const float *const p0 = positions + 3 * index[corner[0]], *const p1 = positions + 3 * index[corner[1]],
                                  *const p2 = positions + 3 * index[corner[2]];
                const X87Reg third = 0.333333343f;   // (0x6fe780)
                cx = (float)((X87Reg(p2[0]) + X87Reg(p0[0]) + X87Reg(p1[0])) * third);
                cy = (float)((X87Reg(p1[1]) + X87Reg(p0[1]) + X87Reg(p2[1])) * third);
                cz = (float)((X87Reg(p2[2]) + X87Reg(p1[2]) + X87Reg(p0[2])) * third);
            }
            for (u32 j = 0; j < 3; ++j) {
                const float u = u_of(part, corner[j]), v = v_of(part, corner[j]);
                if (map && positions) {
                    if (!(cy > -0.100000001f) || !(cy < 0.100000001f)) {
                        ++off;
                        continue;
                    }
                    const s32 z = ftol(X87Reg(cz) * X87Reg(-0.100000001f)), x = ftol(X87Reg(cx) * X87Reg(0.100000001f));
                    if (!map[(s32)((u32)x * 16 - (u32)z)]) {
                        ++off;
                        continue;
                    }
                }
                if (!*warned && outside(u, v))
                    *warned = 1;   // (with Debug_Printf(0x74d624 "Ride %s has an invalid base...", model + 0x18): nothing)
            }
        }
        u16 *const material = at<u16 *>(group, 0);
        if (!off && material && !(*material & 0x8000))
            *material &= 0xffcf;
    }
    GlobalMem_Free(leastV);
    GlobalMem_Free(leastU);
    GlobalMem_Free(done);
}
