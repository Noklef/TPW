// Motion path curves: reimplementation of 0x474840-0x474ea0. See curve.h for why each
// component keeps its own association.
#include "curve.h"

#include <cmath>

namespace {

struct Segment {
    const Vec3 &a, &b, &c, &d;
};

// The four points of segment start k - 1 .. k + 2, wrapped (k + count - 1 stays unsigned).
Segment wrapped(const Vec3 *p, u32 count, s32 k) {
    u32 at = (u32)k + count;
    return {p[(at - 1) % count], p[at % count], p[(at + 1) % count], p[(at + 2) % count]};
}

const Vec3 *own_points(const void *curve, const Vec3 *points) {
    return points ? points : *reinterpret_cast<const Vec3 *const *>(static_cast<const u8 *>(curve) + 8);
}

u32 game_point_count() { return *reinterpret_cast<const u32 *>(0x007b49e8); }  // gCurvePointCount

}  // namespace

extern "C" void Curve_BezierAt(const Vec3 *points, u32 count, s32 base, float t, Vec3 *out) {
    s32 i = (s32)t;
    float f = t - (float)i;
    Segment s = wrapped(points, count, base + 3 * i);
    const Vec3 &a = s.a, &b = s.b, &c = s.c, &d = s.d;
    out->x = (((d.x - ((c.x * 3.0f + a.x * 1.0f) + b.x * -3.0f)) * f + (b.x * -6.0f - (c.x + a.x) * -3.0f)) * f
              + (b.x * 3.0f - a.x * 3.0f)) * f + a.x;
    out->y = (((((d.y - b.y * -3.0f) - a.y) - c.y * 3.0f) * f + (b.y * -6.0f - (c.y + a.y) * -3.0f)) * f
              + (a.y * -3.0f - b.y * -3.0f)) * f + a.y;
    out->z = (((((d.z - c.z * 3.0f) - b.z * -3.0f) - a.z) * f + (b.z * -6.0f - (a.z + c.z) * -3.0f)) * f
              + (a.z * -3.0f - b.z * -3.0f)) * f + a.z;
}

extern "C" void Curve_BezierTangentAt(const Vec3 *points, u32 count, s32 base, float t, Vec3 *out) {
    s32 i = (s32)t;
    float f = t - (float)i;
    Segment s = wrapped(points, count, base + 3 * i);
    const Vec3 &a = s.a, &b = s.b, &c = s.c, &d = s.d;
    out->x = (b.x * 3.0f - a.x * 3.0f)
             - ((b.x * -6.0f - (a.x + c.x) * -3.0f) * -2.0f - ((d.x - ((c.x * 3.0f + a.x * 1.0f) + b.x * -3.0f)) * f) * 3.0f) * f;
    out->y = (a.y * -3.0f - b.y * -3.0f)
             - ((b.y * -6.0f - (a.y + c.y) * -3.0f) * -2.0f - ((((d.y - b.y * -3.0f) - a.y) - c.y * 3.0f) * f) * 3.0f) * f;
    out->z = (a.z * -3.0f - b.z * -3.0f)
             - ((b.z * -6.0f - (c.z + a.z) * -3.0f) * -2.0f - ((((d.z - b.z * -3.0f) - a.z) - c.z * 3.0f) * f) * 3.0f) * f;
}

extern "C" void Curve_LinearAt(const Vec3 *points, s32 base, float t, Vec3 *out) {
    s32 i = (s32)t;
    float f = t - (float)i, g = 1.0f - f;
    const Vec3 &p0 = points[base + i], &p1 = points[base + i + 1];
    out->x = p0.x * g + p1.x * f;
    out->y = p0.y * g + p1.y * f;
    out->z = p0.z * g + p1.z * f;
}

extern "C" void Curve_LinearTangentAt(const Vec3 *points, s32 base, float t, Vec3 *out) {
    s32 i = (s32)t;
    const Vec3 &p0 = points[base + i], &p1 = points[base + i + 1];
    out->x = p1.x - p0.x;
    out->y = p1.y - p0.y;
    out->z = p1.z - p0.z;
}

extern "C" void Curve_CatmullRomAt(const Vec3 *points, u32 count, s32 base, float t, Vec3 *out) {
    s32 i = (s32)t;
    float f = t - (float)i;
    Segment s = wrapped(points, count, base + i);
    const Vec3 &a = s.a, &b = s.b, &c = s.c, &d = s.d;
    out->x = ((((d.x - ((c.x * 3.0f + a.x * 1.0f) + b.x * -3.0f)) * f + (((a.x + a.x) - c.x * -4.0f) - b.x * 5.0f - d.x)) * f
               + (c.x - a.x)) * f + (b.x + b.x)) * 0.5f;
    out->y = ((((((d.y - c.y * 3.0f) - b.y * -3.0f) - a.y) * f + (((b.y * -5.0f - c.y * -4.0f) - a.y * -2.0f) - d.y)) * f
               + (c.y - a.y)) * f + (b.y + b.y)) * 0.5f;
    out->z = ((((((d.z - b.z * -3.0f) - a.z) - c.z * 3.0f) * f + (((a.z + a.z) - b.z * 5.0f) - c.z * -4.0f - d.z)) * f
               + (c.z - a.z)) * f + (b.z + b.z)) * 0.5f;
}

extern "C" void __cdecl Curve_Bezier(const void *curve, const Vec3 *points, s32 base, float t, Vec3 *out) {
    Curve_BezierAt(own_points(curve, points), game_point_count(), base, t, out);
}
extern "C" void __cdecl Curve_BezierTangent(const void *curve, const Vec3 *points, s32 base, float t, Vec3 *out) {
    Curve_BezierTangentAt(own_points(curve, points), game_point_count(), base, t, out);
}
extern "C" void __cdecl Curve_Linear(const void *curve, const Vec3 *points, s32 base, float t, Vec3 *out) {
    Curve_LinearAt(own_points(curve, points), base, t, out);
}
// Unlike the others, the original ignores the points argument here.
extern "C" void __cdecl Curve_LinearTangent(const void *curve, const Vec3 * /*points*/, s32 base, float t, Vec3 *out) {
    Curve_LinearTangentAt(own_points(curve, nullptr), base, t, out);
}
extern "C" void __cdecl Curve_CatmullRom(const void *curve, const Vec3 *points, s32 base, float t, Vec3 *out) {
    Curve_CatmullRomAt(own_points(curve, points), game_point_count(), base, t, out);
}

// modf and fmod are the CRT's (0x67b280, 0x67b24a), both exact, so the double steps match.
extern "C" void __cdecl Path_PercentAt(const float *table, u32 count, float t, float *out, s32 closed) {
    double whole;
    float f = (float)std::modf((double)t, &whole);
    u32 i = (u32)(s32)whole;
    if (i == count) {
        f = f - -1.0f;
        --i;
    }
    float a, b;
    if (closed) {
        a = table[i];
        b = table[(i + 1) % count];
    } else {
        a = table[i];
        b = i < count - 1 ? table[i + 1] : table[i];
    }
    if (std::fabs(a - b) > 50.0f) {
        if (a < b) a = a - -100.0f;
        else b = b - -100.0f;
        *out = (float)std::fmod((double)((1.0f - f) * a + f * b), 100.0);
        return;
    }
    *out = (1.0f - f) * a + f * b;
}
