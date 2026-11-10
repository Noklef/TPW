#include "carprisms.h"

#include <cmath>
#include <cstdint>
#include <emmintrin.h>

#include "../math/x87.h"
#include "helpers.h"
#include "../math/sintable.h"
#include "contacts.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const float kOne = 1.0f;                // 0x6fdf70
const float kZero = 0.0f;               // 0x6fdf74
const float kMinusHalfPi = -1.57079637f;// 0x6fdf78
const float kFullTurn = 6.28218746f;    // 0x6fdf7c

// The game's acos (0x67b4e0) of a register, as in terrain/raypick.
double CrtAcos(X87Reg x) {
    const X87Reg r = X87_Sqrt((X87Reg(1.0) + x) * (X87Reg(1.0) - x));
    return X87_Atan2(r.v, x.v);
}

// The sine table's entry for a value stored as a float first (fistp, rounded to nearest).
float Entry(X87Reg scaled) {
    const float index = (float)scaled;
    return reinterpret_cast<const float *>(0x007b0968)[_mm_cvtss_si32(_mm_set_ss(index)) & gSinTableMask];
}

// One point (x, y, 0) turned by the three angles (each with its less -pi/2) and moved to the position.
void Place(float *out, const float *point, const float *position, const float *angles, const float *shifted) {
    const float scale = gSinTableScale;
    at<u32>(out, 0) = at<u32>(point, 0);
    at<u32>(out, 4) = at<u32>(point, 4);
    at<u32>(out, 8) = 0;
    {
        const float s = Entry(X87Reg(angles[2]) * scale), c = Entry(X87Reg(scale) * shifted[2]);
        const X87Reg x(out[0]);
        out[0] = (float)(x * X87Reg(c) - X87Reg(s) * out[1]);
        out[1] = (float)(X87Reg(c) * out[1] + x * X87Reg(s));
    }
    {
        const float s = Entry(X87Reg(angles[1]) * scale), c = Entry(X87Reg(scale) * shifted[1]);
        const X87Reg y(out[1]);
        out[1] = (float)(y * X87Reg(c) - X87Reg(s) * out[2]);
        out[2] = (float)(X87Reg(c) * out[2] + y * X87Reg(s));
    }
    {
        const float s = Entry(X87Reg(angles[0]) * scale), c = Entry(X87Reg(scale) * shifted[0]);
        const X87Reg x(out[0]);
        out[0] = (float)(X87Reg(s) * out[2] + x * X87Reg(c));
        out[2] = (float)(X87Reg(c) * out[2] - x * X87Reg(s));
    }
    out[0] = (float)(X87Reg(position[0]) + out[0]);
    out[1] = (float)(X87Reg(out[1]) + position[1]);
    out[2] = (float)(X87Reg(out[2]) + position[2]);
}

// The vector stored, then over its length (in registers).
void Unit(float *v) {
    const X87Reg length = X87_Sqrt(((X87Reg(v[0]) * X87Reg(v[0])) + (X87Reg(v[1]) * v[1])) + (X87Reg(v[2]) * v[2]));
    v[0] = (float)(X87Reg(v[0]) / length);
    v[1] = (float)(X87Reg(v[1]) / length);
    v[2] = (float)(X87Reg(v[2]) / length);
}

float *Corner(u32 base, u32 i) { return reinterpret_cast<float *>((uintptr_t)base + i * 12); }

// The two rows of places for the shape at list and car; the count read for each row from the first shape (a shape of seven points writes over 0x7922dc, sic).
void PlaceShape(const u8 *list, const u8 *car, float *out) {
    const float angles[3] = {at<float>(car, 0x10), at<float>(car, 0x14), at<float>(car, 0x18)};
    const float shifted[3] = {(float)(X87Reg(angles[0]) - kMinusHalfPi), (float)(X87Reg(angles[1]) - kMinusHalfPi), (float)(X87Reg(angles[2]) - kMinusHalfPi)};
    for (u32 row = 0; row < 2; ++row) {
        const float *const position = reinterpret_cast<const float *>(car + 4 + row * 0x38);
        const float *point = at<const float *>(list, 0);
        const u32 count = at<u32>(global<const u8 *>(0x007922dc), 4);
        for (u32 k = 0; k < count; ++k, point += 2, out += 3) Place(out, point, position, angles, shifted);
    }
}

}  // namespace

extern "C" u32 __cdecl CarPrisms_InQuad(const float *point, float t, const float *way) {
    const X87Reg tx = X87Reg(t) * way[0], ty = X87Reg(way[1]) * t, tz = X87Reg(way[2]) * t;
    const float x = (float)(tx + point[0]), y = (float)(ty + point[1]), z = (float)(tz + point[2]);
    const float *const c[4] = {global<const float *>(0x0079238c), global<const float *>(0x00792384), global<const float *>(0x00792388), global<const float *>(0x00792380)};
    float v[4][3];
    float length[4];
    for (int k = 0; k < 4; ++k) {
        const X87Reg vx = X87Reg(c[k][0]) - x;
        v[k][0] = (float)vx;
        v[k][1] = (float)(X87Reg(c[k][1]) - y);
        v[k][2] = (float)(X87Reg(c[k][2]) - z);
        length[k] = (float)X87_Sqrt(((vx * vx) + (X87Reg(v[k][1]) * v[k][1])) + (X87Reg(v[k][2]) * v[k][2]));
        for (int j = 0; j < 3; ++j) v[k][j] = (float)(X87Reg(v[k][j]) / length[k]);
    }
    const double a23 = CrtAcos(((X87Reg(v[1][1]) * v[2][1]) + (X87Reg(v[1][2]) * v[2][2])) + (X87Reg(v[2][0]) * v[1][0]));
    const double a12 = CrtAcos(((X87Reg(v[1][2]) * v[0][2]) + (X87Reg(v[1][1]) * v[0][1])) + (X87Reg(v[1][0]) * v[0][0]));
    const X87Reg first = X87Reg(a23) + X87Reg(a12);
    const double a34 = CrtAcos(((X87Reg(v[2][1]) * v[3][1]) + (X87Reg(v[3][2]) * v[2][2])) + (X87Reg(v[3][0]) * v[2][0]));
    const float three = (float)(first + X87Reg(a34));
    const double a41 = CrtAcos(((X87Reg(v[3][1]) * v[0][1]) + (X87Reg(v[3][2]) * v[0][2])) + (X87Reg(v[3][0]) * v[0][0]));
    return (X87Reg(a41) + three).v >= kFullTurn;
}

extern "C" u32 __cdecl CarPrisms_Touch() {
    PlaceShape(global<const u8 *>(0x007922dc), gContacts_79223c, Corner(0x007922f0, 0));
    PlaceShape(gContacts_792244, gContacts_792240, Corner(0x00792248, 0));

    const u32 n = at<u32>(global<const u8 *>(0x007922dc), 4);
    for (u32 i = 0; i < n; ++i) {
        const float *const a = Corner(0x007922f0, i), *const b = Corner(0x007922f0, (i + 1) % n);
        const float *const c = Corner(0x007922f0, n + i), *const d = Corner(0x007922f0, (i + 1) % n + n);
        global<const float *>(0x0079238c) = a;
        global<const float *>(0x00792384) = b;
        global<const float *>(0x00792388) = c;
        global<const float *>(0x00792380) = d;
        const X87Reg e1x = X87Reg(b[0]) - a[0], e1y = X87Reg(b[1]) - a[1], e1z = X87Reg(b[2]) - a[2];
        const float e2x = (float)(X87Reg(b[0]) - c[0]), e2y = (float)(X87Reg(b[1]) - c[1]);
        const X87Reg e2z = X87Reg(b[2]) - c[2];
        float nx = (float)(e2z * e1y - X87Reg(e2y) * e1z);
        float ny = (float)(X87Reg(e2x) * e1z - e2z * e1x);
        const X87Reg nzr = e1x * X87Reg(e2y) - X87Reg(e2x) * e1y;
        const X87Reg length = X87_Sqrt(((X87Reg(nx) * X87Reg(nx)) + (X87Reg(ny) * ny)) + nzr * nzr);
        nx = (float)(X87Reg(nx) / length);
        ny = (float)(X87Reg(ny) / length);
        const float nz = (float)(nzr / length);
        const X87Reg through = ((X87Reg(b[0]) * nx) + (X87Reg(b[1]) * ny)) + (X87Reg(b[2]) * nz);
        const float d0 = (float)X87Reg(-through.v);

        const u32 m = at<u32>(gContacts_792244, 4);
        for (u32 j = 0; j < m; ++j) {
            const float *const p = Corner(0x00792248, j), *const q = Corner(0x00792248, (j + 1) % m), *const r = Corner(0x00792248, m + j);
            const float along[3] = {(float)(X87Reg(q[0]) - p[0]), (float)(X87Reg(q[1]) - p[1]), (float)(X87Reg(q[2]) - p[2])};
            const float across[3] = {(float)(X87Reg(r[0]) - p[0]), (float)(X87Reg(r[1]) - p[1]), (float)(X87Reg(r[2]) - p[2])};
            const X87Reg height = (((X87Reg(nz) * p[2]) + (X87Reg(ny) * p[1])) + (X87Reg(nx) * p[0])) + d0;
            const X87Reg under1 = ((X87Reg(nx) * along[0]) + (X87Reg(ny) * along[1])) + (X87Reg(nz) * along[2]);
            const float t1 = (float)X87Reg(-(height / under1).v);
            const X87Reg under2 = ((X87Reg(nx) * across[0]) + (X87Reg(ny) * across[1])) + (X87Reg(nz) * across[2]);
            const float t2 = (float)X87Reg(-(height / under2).v);

            if (t1 >= kZero && t1 <= kOne) {
                const X87Reg ix = (X87Reg(t1) * along[0]) + p[0];
                const float iy = (float)((X87Reg(t1) * along[1]) + p[1]), iz = (float)((X87Reg(t1) * along[2]) + p[2]);
                float va[3] = {(float)(X87Reg(a[0]) - ix), (float)(X87Reg(a[1]) - iy), (float)(X87Reg(a[2]) - iz)};
                float vb[3] = {(float)(X87Reg(b[0]) - ix), (float)(X87Reg(b[1]) - iy), (float)(X87Reg(b[2]) - iz)};
                float vc[3] = {(float)(X87Reg(c[0]) - ix), (float)(X87Reg(c[1]) - iy), (float)(X87Reg(c[2]) - iz)};
                float vd[3] = {(float)(X87Reg(d[0]) - ix), (float)(X87Reg(d[1]) - iy), (float)(X87Reg(d[2]) - iz)};
                Unit(va);
                Unit(vb);
                Vec3_Normalize(vc);
                Vec3_Normalize(vd);
                float sum = (float)CrtAcos(X87Reg(Vec3_Dot(va, vb)));
                sum = (float)(X87Reg(CrtAcos(X87Reg(Vec3_Dot(vb, vc)))) + sum);
                sum = (float)(X87Reg(CrtAcos(X87Reg(Vec3_Dot(vc, vd)))) + sum);
                if ((X87Reg(CrtAcos(X87Reg(Vec3_Dot(vd, va)))) + sum).v >= kFullTurn) return 1;
            }
            if (t2 >= kZero && t2 <= kOne && CarPrisms_InQuad(p, t2, across)) return 1;
        }
    }
    return 0;
}
