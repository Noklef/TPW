#include "nodeangles.h"

#include <emmintrin.h>

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "helpers.h"
#include "../math/sintable.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const float kOne = 1.0f;              // 0x6fdf28
const float kMinusHalfPi = -1.57079637f;   // 0x6fdf2c
const float kMinusOne = -1.0f;        // 0x6fdf30
const float kZero = 0.0f;             // 0x6fdf34
const u32 kNode = 0x38;

u32 Ftol(X87Reg x) { return (u32)(s32)(long long)x.v; }
s32 Fistp(float x) { return _mm_cvtss_si32(_mm_set_ss(x)); }
float Table(s32 i) { return global<float>(0x007b0968 + 4 * (gSinTableMask & (u32)i)); }

// The table entries of -angle: its sine's and its cosine's (a quarter turn on).
void Lookup(double angle, float &sine, float &cosine) {
    const float a = (float)-angle;
    const float scale = gSinTableScale;
    const s32 i = Fistp((float)(X87Reg(scale) * a));
    const s32 j = Fistp((float)((X87Reg(a) - kMinusHalfPi) * scale));
    sine = Table(i);
    cosine = Table(j);
}

// The game's acos (0x67b4e0), as in motion/quat.cpp.
double CrtAcos(float x) {
    const float r = std::sqrt((1.0f + x) * (1.0f - x));
    return std::atan2((double)r, (double)x);
}

// fcomp 0: above. fcomp 1: below or unordered.
float Clamped(float c) {
    if (kZero > c) c = 0.0f;
    if (!(kOne >= c)) c = 1.0f;
    return c;
}

float Weight(const u8 *path, const u8 *node, float s, float t) {
    const u8 *params = at<u8 *>(path, 4);
    const float a = at<u8>(node, 0) & 8 ? at<float>(params, 0x5c) : at<float>(params, 0x58);
    const float b = at<u8>(node, kNode) & 8 ? at<float>(params, 0x5c) : at<float>(params, 0x58);
    return (float)(X87Reg(a) * s + X87Reg(b) * t);
}

struct Place {
    float s, t;
    float x, y, z;       // the place
    float ax, az;        // the car's vector, flat
    float by, bz;        // the car's vector, turned
};

// One row. Ahead: out2's sign test is the other way round.
void Row(const Place &p, const u8 *path, const u8 *node, float *out1, float *out2, float *out3, bool ahead) {
    float d[3];
    d[0] = (float)(X87Reg(at<float>(node, 0x3c)) * p.t + X87Reg(at<float>(node, 4)) * X87Reg(p.s) - p.x);
    d[1] = (float)(X87Reg(at<float>(node, 0x40)) * p.t + X87Reg(at<float>(node, 8)) * X87Reg(p.s) - p.y);
    d[2] = (float)(X87Reg(at<float>(node, 0x44)) * p.t + X87Reg(at<float>(node, 0xc)) * X87Reg(p.s) - p.z);

    X87Reg length = X87_Sqrt(X87Reg(d[0]) * X87Reg(d[0]) + X87Reg(d[2]) * d[2]);
    const float dx = (float)(X87Reg(d[0]) / length), dz = (float)(X87Reg(d[2]) / length);

    float sine, cosine;
    Lookup(Vec3_AngleY(d), sine, cosine);
    const X87Reg turned = X87Reg(cosine) * d[2] - X87Reg(d[0]) * X87Reg(sine);
    length = X87_Sqrt(X87Reg(d[1]) * X87Reg(d[1]) + turned * turned);
    const float ey = (float)(X87Reg(d[1]) / length), ez = (float)(turned / length);

    const double first = CrtAcos(Clamped((float)(X87Reg(p.ax) * dx + X87Reg(p.az) * dz)));
    const X87Reg cross1 = X87Reg(p.ax) * dz - X87Reg(p.az) * dx;
    *out1 = cross1.v > kZero ? (float)(X87Reg(first) * kMinusOne) : (float)first;

    const double second = CrtAcos(Clamped((float)(X87Reg(p.by) * ey + X87Reg(p.bz) * ez)));
    const X87Reg cross2 = X87Reg(p.by) * ez - X87Reg(p.bz) * ey;
    const bool negate = ahead ? !(cross2.v >= kZero) : cross2.v > kZero;
    *out2 = negate ? (float)(X87Reg(second) * kMinusOne) : (float)second;

    *out3 = Weight(path, node, p.s, p.t);
}

void Negate(float &x) { x = (float)(X87Reg(x) * kMinusOne); }

}  // namespace

void __cdecl Coaster_NodeAngles(const u8 *path, const u8 *car, float *out1, float *out2, float *out3, u32, float position, u32 *count) {
    const u8 *params = at<u8 *>(car, 4);
    const u8 *nodes = at<u8 *>(path, 0x9c);

    double whole;   // (CRT modf 0x67b280)
    const float t = (float)std::modf((double)position, &whole);
    const u32 index = Ftol(whole);

    const float spacing = at<float>(path, 0xac);
    const u32 behindWanted = Ftol(X87Reg(at<float>(params, 0x28)) / X87Reg(spacing)) + 1;
    const u32 behind = behindWanted > index ? index : behindWanted;
    const u32 aheadWanted = Ftol(X87Reg(at<float>(params, 0x24)) / X87Reg(spacing)) + 1;
    const u32 left = at<u32>(path, 0xa4) - 1 - index;
    const u32 ahead = aheadWanted > left ? left : aheadWanted;

    const u8 *node = nodes + index * kNode;
    Place p;
    const X87Reg s = X87Reg(kOne) - t;
    p.s = (float)s;
    p.t = t;
    p.x = (float)(X87Reg(at<float>(node, 4)) * s + X87Reg(at<float>(node, 0x3c)) * t);
    p.y = (float)(X87Reg(at<float>(node, 8)) * s + X87Reg(at<float>(node, 0x40)) * t);
    p.z = (float)(X87Reg(at<float>(node, 0xc)) * s + X87Reg(at<float>(node, 0x44)) * t);

    float heading[3];
    std::memcpy(heading, car + 0x10, sizeof heading);
    const double angle = Angle_Lerp(at<float>(node, 0x10), at<float>(node, 0x48), t);

    X87Reg length = X87_Sqrt(X87Reg(heading[0]) * X87Reg(heading[0]) + X87Reg(heading[2]) * heading[2]);
    p.ax = (float)(X87Reg(heading[0]) / length);
    p.az = (float)(X87Reg(heading[2]) / length);

    float sine, cosine;
    Lookup(angle, sine, cosine);
    const X87Reg turned = X87Reg(cosine) * heading[2] - X87Reg(sine) * heading[0];
    length = X87_Sqrt(X87Reg(heading[1]) * X87Reg(heading[1]) + turned * turned);
    p.by = (float)(X87Reg(heading[1]) / length);
    p.bz = (float)(turned / length);

    for (u32 k = behind; k < behindWanted; ++k) {
        *out1++ = 0.0f;
        *out2++ = 0.0f;
        *out3++ = 0.0f;
    }

    if (behind != 0) {
        Place back = p;   // (the original negates these in place and back again)
        Negate(back.ax);
        Negate(back.az);
        Negate(back.by);
        const u8 *row = nodes + (index - behind) * kNode;
        for (u32 k = 0; k < behind; ++k, row += kNode) Row(back, path, row, out1++, out2++, out3++, false);
    }

    *out1++ = 0.0f;
    *out2++ = 0.0f;
    *out3++ = Weight(path, node, p.s, t);

    if (ahead != 0 && index + 1 <= index + ahead) {
        const u8 *row = nodes + (index + 1) * kNode;
        for (u32 k = 0; k < ahead; ++k, row += kNode) Row(p, path, row, out1++, out2++, out3++, true);
    }

    for (u32 k = ahead; k < aheadWanted; ++k) {
        *out1++ = 0.0f;
        *out2++ = 0.0f;
        *out3++ = 0.0f;
    }

    *count = behindWanted;
}
