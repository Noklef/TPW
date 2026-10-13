// Quaternion helpers: reimplementation of 0x474120-0x474575.
// The expression trees are the original's (tools/x87sym.py); Ghidra's C shows a different
// association.
#include "quat.h"

#include <cmath>

namespace {

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
    const float x = q[0], y = q[1], z = q[2], w = q[3];
    const float s = 2.0f / (((y * y + z * z) + w * w) + x * x);
    const float sy = s * y, sz = s * z, xs = x * s;
    m[0] = 1.0f - (sz * z + sy * y);
    m[1] = x * sy + sz * w;
    m[2] = x * sz - sy * w;
    m[3] = x * sy - sz * w;
    m[4] = 1.0f - (sz * z + x * xs);
    m[5] = sz * y + xs * w;
    m[6] = x * sz + sy * w;
    m[7] = sz * y - xs * w;
    m[8] = 1.0f - (sy * y + x * xs);
}

void Quat_SlerpWith(const SinTable &sines, const float *a, const float *b, float t, float *out) {
    const float dot = ((b[3] * a[3] + b[2] * a[2]) + b[1] * a[1]) + b[0] * a[0];
    if (!(dot - -1.0f > 0.001f)) {
        // Nearly opposite: turn a by up to 90 degrees towards (-y, x, -w, z). Written in the
        // original's order (out may be a).
        out[0] = -a[1];
        out[1] = a[0];
        out[2] = -a[3];
        out[3] = a[2];
        const float s0 = SinTable_At(sines, ((1.0f - t) * kHalfPi) * sines.scale);
        const float s1 = SinTable_At(sines, (t * kHalfPi) * sines.scale);
        out[0] = out[0] * s1 + a[0] * s0;
        out[1] = s1 * out[1] + s0 * a[1];
        out[2] = s1 * out[2] + s0 * a[2];
        return;
    }
    float s0, s1;
    if (!(1.0f - dot > 0.001f)) {
        s0 = 1.0f - t;
        s1 = t;
    } else {
        const double theta = crt_acos(dot);
        const float theta_f = (float)theta;
        const float sin_theta = SinTable_At(sines, (float)(theta * sines.scale));  // the double times scale
        s0 = SinTable_At(sines, ((1.0f - t) * theta_f) * sines.scale) / sin_theta;
        s1 = SinTable_At(sines, (theta_f * t) * sines.scale) / sin_theta;
    }
    out[0] = a[0] * s0 + b[0] * s1;
    out[1] = s0 * a[1] + s1 * b[1];
    out[2] = s1 * b[2] + s0 * a[2];
    out[3] = s0 * a[3] + s1 * b[3];
}

void Motion_RotationAtWith(const SinTable &sines, const RotationKey *keys, u32 count, u32 flags, u32 index,
                           float t, float *m) {
    const u32 next = index < count - 1 ? index + 1 : index;
    const float *a = keys[index].q, *b = keys[next].q;
    float q[4];
    if (flags & 2) {
        const float u = 1.0f - t;
        q[0] = t * b[0] + u * a[0];
        q[1] = a[1] * u + b[1] * t;
        q[2] = a[2] * u + b[2] * t;
        q[3] = a[3] * u + b[3] * t;
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
