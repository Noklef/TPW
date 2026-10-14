#include "spline.h"

#include "../math/x87.h"

namespace {

struct Points {
    X87Reg a, b, c, d;  // from.position, from.ahead, to.behind, to.position
};

Points component(const u8 *from, const u8 *to, u32 axis) {
    auto f = [](const u8 *node, u32 offset) { return (double)*reinterpret_cast<const float *>(node + offset); };
    return {f(from, 0x40 + axis), f(from, 0x4c + axis), f(to, 0x34 + axis), f(to, 0x40 + axis)};
}

}  // namespace

extern "C" {

void __cdecl TrackSpline_Point(const u8 *from, const u8 *to, float t, Vec3 *out) {
    const X87Reg u = t;
    {
        const Points p = component(from, to, 0);
        out->x = (float)((((p.d - ((p.c * 3.0 + p.b * -3.0) + p.a)) * u + (p.b * -6.0 - (p.a + p.c) * -3.0)) * u +
                          (p.a * -3.0 - p.b * -3.0)) * u + p.a);
    }
    {
        const Points p = component(from, to, 4);
        out->y = (float)((((((p.b * 3.0 - p.a) - p.c * 3.0) + p.d) * u + (p.b * -6.0 - (p.c + p.a) * -3.0)) * u +
                          (p.b * 3.0 - p.a * 3.0)) * u + p.a);
    }
    {
        const Points p = component(from, to, 8);
        out->z = (float)((((((p.b * 3.0 - p.a) - p.c * 3.0) + p.d) * u + (p.b * -6.0 - (p.a + p.c) * -3.0)) * u +
                          (p.b * 3.0 - p.a * 3.0)) * u + p.a);
    }
}

void __cdecl TrackSpline_Tangent(const u8 *from, const u8 *to, float t, Vec3 *out) {
    const X87Reg u = t;
    {
        const Points p = component(from, to, 0);
        out->x = (float)((p.a * -3.0 - p.b * -3.0) -
                         ((p.b * -6.0 - (p.c + p.a) * -3.0) * -2.0 - ((p.d - ((p.c * 3.0 + p.b * -3.0) + p.a)) * u) * 3.0) * u);
    }
    {
        const Points p = component(from, to, 4);
        out->y = (float)((p.a * -3.0 - p.b * -3.0) -
                         ((p.b * -6.0 - (p.c + p.a) * -3.0) * -2.0 - ((p.d - ((p.c * 3.0 + p.b * -3.0) + p.a)) * u) * 3.0) * u);
    }
    {
        const Points p = component(from, to, 8);
        out->z = (float)((p.b * 3.0 - p.a * 3.0) -
                         ((p.b * -6.0 - (p.c + p.a) * -3.0) * -2.0 - (((((p.b * 3.0 - p.a) - p.c * 3.0) + p.d) * u) * 3.0)) * u);
    }
}
}
