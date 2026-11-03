#include "rectclip.h"

#include <cstring>

#include "geometry.h"

namespace {

struct Point {
    float x, y, z;
};

const u32 kCount = 0xd0;

u32 &count_of(u8 *polygon) { return *reinterpret_cast<u32 *>(polygon + kCount); }

u32 side(float v, float low, float high) { return (v > low ? 1u : 0u) + (v > high ? 1u : 0u); }

void cross_z(Point &out, const Point &a, const Point &b, float z) {
    u32 bits;
    std::memcpy(&bits, &z, 4);
    Segment_AtZ(&out.x, a.x, a.y, a.z, b.x, b.y, b.z, bits);
}

void cross_x(Point &out, const Point &a, const Point &b, float x) { Segment_AtX(&out.x, a.x, a.y, a.z, b.x, b.y, b.z, x); }

// One pass: from's edges (each point and the one before it) into to; returns how many.
template <void (*Cross)(Point &, const Point &, const Point &, float)>
u32 clip_pass(u8 *polygon, const Point *from, Point *to, float Point::*axis, float low, float high) {
    u32 n = 0;
    u32 previous = count_of(polygon) - 1;
    for (u32 i = 0; i != count_of(polygon); previous = i, i++) {
        const Point &start = from[previous], &end = from[i];
        const auto keep = [&] { to[n++] = start; };
        const auto cross = [&](float limit) {
            Point p;
            Cross(p, end, start, limit);
            to[n++] = p;
        };
        switch (side(end.*axis, low, high) * 3 + side(start.*axis, low, high)) {
        case 1: keep(); cross(low); break;
        case 2: cross(high); cross(low); break;
        case 3: cross(low); break;
        case 4: keep(); break;
        case 5: cross(high); break;
        case 6: cross(low); cross(high); break;
        case 7: keep(); cross(high); break;
        default: break;
        }
    }
    return n;
}

}  // namespace

extern "C" void __fastcall Polygon_ClipToRect(u8 *polygon, void *, float x0, float x1, float z0, float z1) {
    Point *const points = reinterpret_cast<Point *>(polygon);
    Point middle[16];
    count_of(polygon) = clip_pass<cross_z>(polygon, points, middle, &Point::z, z0, z1);
    count_of(polygon) = clip_pass<cross_x>(polygon, middle, points, &Point::x, x0, x1);
}
