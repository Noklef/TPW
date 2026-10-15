// Quaternion helpers: reimplementation of 0x474120-0x474575.
// The expression trees are the original's (tools/x87sym.py); Ghidra's C shows a different
// association.
#include "quat.h"

#include <cmath>

#include "../math/x87.h"

namespace {

using R = X87Reg;

const float kHalfPi = 1.57079637f;  // 0x6fec74

// The CRT's acos (0x67b4e0) as the game calls it. It keeps the caller's precision control,
// so under Direct3D's single precision 1 + x, 1 - x, their product and the square root all
// round to float; fpatan is exempt, and its result leaves through the CRT's error check,
// which stores it as a double. Only called with |x| < 1.
double crt_acos(float x) {
    const float r = std::sqrt((1.0f + x) * (1.0f - x));
    return std::atan2((double)r, (double)x);
}

}  // namespace

extern "C" void __cdecl Quat_ToMatrix(const float *q, float *m) {
    // Which products are stored as floats and which stay in the registers is the original's.
    const float x = q[0], y = q[1], z = q[2], w = q[3];
    const R s = R(2.0f) / (((R(y) * y + R(z) * z) + R(w) * w) + R(x) * x);
    const float sf = (float)s;
    const float xs = (float)(R(x) * sf);
    const R sy_r = R(sf) * y, sz_r = s * z;  // sic: z's scale is the register's s, y's the float's
    const float sy = (float)sy_r, sz = (float)sz_r;
    const R xsw = R(xs) * w, xsy = R(x) * sy, xsz = R(x) * sz;
    const float xxs = (float)(R(x) * xs), szz = (float)(sz_r * z), syy = (float)(sy_r * y);
    const float syw = (float)(R(sy) * w), szw = (float)(R(sz) * w), szy = (float)(R(sz) * y);
    m[0] = (float)(R(1.0f) - (R(szz) + syy));
    m[1] = (float)(xsy + szw);
    m[2] = (float)(xsz - syw);
    m[3] = (float)(R((float)xsy) - szw);
    m[4] = (float)(R(1.0f) - (R(szz) + xxs));
    m[5] = (float)(R(szy) + xsw);
    m[6] = (float)(R((float)xsz) + syw);
    m[7] = (float)(R(szy) - xsw);
    m[8] = (float)(R(1.0f) - (R(syy) + xxs));
}

void Quat_SlerpWith(const SinTable &sines, const float *a, const float *b, float t, float *out) {
    const R dot_r = ((R(b[3]) * a[3] + R(b[2]) * a[2]) + R(b[1]) * a[1]) + R(b[0]) * a[0];
    const float dot = (float)dot_r;  // the first test takes the register's
    if (!((dot_r - -1.0f).v > 0.001f)) {
        // Nearly opposite: turn a by up to 90 degrees towards (-y, x, -w, z). Written in the
        // original's order (out may be a).
        out[0] = -a[1];
        out[1] = a[0];
        out[2] = -a[3];
        out[3] = a[2];
        const float s0 = SinTable_At(sines, (float)((R(1.0f) - t) * kHalfPi * sines.scale));
        const float s1 = SinTable_At(sines, (float)(R(t) * kHalfPi * sines.scale));
        out[0] = (float)(R(out[0]) * s1 + R(a[0]) * s0);
        out[1] = (float)(R(s1) * out[1] + R(s0) * a[1]);
        out[2] = (float)(R(s1) * out[2] + R(s0) * a[2]);
        return;
    }
    float s0;
    R s1 = 0.0;  // stays in the registers
    if (!((R(1.0f) - dot).v > 0.001f)) {
        s0 = (float)(R(1.0f) - t);
        s1 = t;
    } else {
        const double theta = crt_acos(dot);
        const float theta_f = (float)theta;
        const float sin_theta = SinTable_At(sines, (float)(theta * sines.scale));  // the double times scale
        s0 = (float)(R(SinTable_At(sines, (float)((R(1.0f) - t) * theta_f * sines.scale))) / sin_theta);
        s1 = R(SinTable_At(sines, (float)(R(theta_f) * t * sines.scale))) / sin_theta;
    }
    out[0] = (float)(R(a[0]) * s0 + R(b[0]) * s1);
    out[1] = (float)(R(s0) * a[1] + s1 * b[1]);
    out[2] = (float)(s1 * b[2] + R(s0) * a[2]);
    out[3] = (float)(R(s0) * a[3] + s1 * b[3]);
}

void Motion_RotationAtWith(const SinTable &sines, const RotationKey *keys, u32 count, u32 flags, u32 index,
                           float t, float *m) {
    const u32 next = index < count - 1 ? index + 1 : index;
    const float *a = keys[index].q, *b = keys[next].q;
    float q[4];
    if (flags & 2) {
        const R u = R(1.0f) - t;
        q[0] = (float)(R(t) * b[0] + u * a[0]);
        q[1] = (float)(R(a[1]) * u + R(b[1]) * t);
        q[2] = (float)(R(a[2]) * u + R(b[2]) * t);
        q[3] = (float)(R(a[3]) * u + R(b[3]) * t);
    } else {
        Quat_SlerpWith(sines, a, b, t, q);
    }
    Quat_ToMatrix(q, m);
}

extern "C" void __cdecl Quat_Slerp(const float *a, const float *b, float t, float *out) {
    Quat_SlerpWith(SinTable_Game(), a, b, t, out);
}

extern "C" void __cdecl Motion_RotationAt(const RotationKey *keys, u32 index, float t, float *m) {
    Motion_RotationAtWith(SinTable_Game(), keys, *reinterpret_cast<const u32 *>(0x007b4988),
                          *reinterpret_cast<const u32 *>(0x007b4980), index, t, m);
}
