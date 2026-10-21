#include "spline.h"

#include <cmath>
#include <cstring>

#include "../math/matrix.h"
#include "../math/sintable.h"
#include "../math/x87.h"

namespace {

const float kMinusHalfPi = -1.57079637f;  // 0x6fe138

float field(const u8 *p, u32 offset) { return *reinterpret_cast<const float *>(p + offset); }

// A node's piece (node +0x10).
const u8 *piece(const u8 *node) { return *reinterpret_cast<const u8 *const *>(node + 0x10); }

// One axis of the four control points, as the x87 code's memory operands (references: a signalling NaN stays one).
struct Points {
    const float &a, &b, &c, &d;  // from.position, from.ahead, to.behind, to.position
};

Points component(const u8 *from, const u8 *to, u32 axis) {
    auto f = [](const u8 *node, u32 offset) -> const float & { return *reinterpret_cast<const float *>(node + offset); };
    return {f(from, 0x40 + axis), f(from, 0x4c + axis), f(to, 0x34 + axis), f(to, 0x40 + axis)};
}

const float k3 = 3.0f, km3 = -3.0f, km6 = -6.0f, km2 = -2.0f;   // 0x6fe110, 0x6fe114, 0x6fe118, 0x6fe11c

}  // namespace

extern "C" {

// The listings' operand forms (x87.h: X87Reg op float is a memory operand, X87Reg(f) a load): a NaN in a register keeps
// its place against a signalling one from memory.
void __cdecl TrackSpline_Point(const u8 *from, const u8 *to, float t, Vec3 *out) {
    {
        const Points p = component(from, to, 0);
        out->x = (float)((((p.d - ((X87Reg(p.c) * k3 + X87Reg(p.b) * km3) + p.a)) * t + (X87Reg(p.b) * km6 - (X87Reg(p.a) + p.c) * km3)) * t +
                          (X87Reg(p.a) * km3 - X87Reg(p.b) * km3)) * t + p.a);
    }
    {
        const Points p = component(from, to, 4);
        out->y = (float)((((((X87Reg(p.b) * k3 - p.a) - X87Reg(p.c) * k3) + p.d) * t + (X87Reg(p.b) * km6 - (X87Reg(p.c) + p.a) * km3)) * t +
                          (X87Reg(p.b) * k3 - X87Reg(p.a) * k3)) * t + p.a);
    }
    {
        const Points p = component(from, to, 8);
        out->z = (float)((((((X87Reg(p.b) * k3 - p.a) - X87Reg(p.c) * k3) + p.d) * t + (X87Reg(p.b) * km6 - (X87Reg(p.a) + p.c) * km3)) * t +
                          (X87Reg(p.b) * k3 - X87Reg(p.a) * k3)) * t + p.a);
    }
}

void __cdecl TrackSpline_Tangent(const u8 *from, const u8 *to, float t, Vec3 *out) {
    {
        const Points p = component(from, to, 0);
        out->x = (float)((X87Reg(p.a) * km3 - X87Reg(p.b) * km3) -
                         ((X87Reg(p.b) * km6 - (X87Reg(p.c) + p.a) * km3) * km2 - ((p.d - ((X87Reg(p.c) * k3 + X87Reg(p.b) * km3) + p.a)) * t) * k3) * t);
    }
    {
        const Points p = component(from, to, 4);
        out->y = (float)((X87Reg(p.a) * km3 - X87Reg(p.b) * km3) -
                         ((X87Reg(p.b) * km6 - (X87Reg(p.c) + p.a) * km3) * km2 - ((p.d - ((X87Reg(p.c) * k3 + X87Reg(p.b) * km3) + p.a)) * t) * k3) * t);
    }
    {
        const Points p = component(from, to, 8);
        out->z = (float)((X87Reg(p.b) * k3 - X87Reg(p.a) * k3) -
                         ((X87Reg(p.b) * km6 - (X87Reg(p.c) + p.a) * km3) * km2 - ((((X87Reg(p.b) * k3 - p.a) - X87Reg(p.c) * k3) + p.d) * t) * k3) * t);
    }
}

void __cdecl TrackSpline_Frame(const u8 *nodes, const u8 *flags, const u8 *from, const u8 *to, const u8 *upFrom,
                               const u8 *upTo, float t, Vec3 *out, TrackOrientation *orient) {
    TrackSpline_Point(from, to, t, out);
    if (!orient) return;
    Vec3 tangent;
    TrackSpline_Tangent(from, to, t, &tangent);
    const float u = (float)(X87Reg(1.0f) - t);
    const float blend = (float)((X87Reg(field(to, 0xc)) + field(piece(to), 0x6c)) * t +
                                (X87Reg(field(from, 0xc)) + field(piece(from), 0x6c)) * u);
    u32 blendBits;
    std::memcpy(&blendBits, &blend, 4);
    TrackOrientation_Set(tangent, blendBits, orient, nullptr);

    // The up spline's point relative to this one, turned back by the yaw, then the pitch.
    Vec3 up;
    TrackSpline_Point(upFrom, upTo, t, &up);
    up.x = (float)(X87Reg(up.x) - out->x);
    up.y = (float)(X87Reg(up.y) - out->y);
    up.z = (float)(X87Reg(up.z) - out->z);
    const SinTable sines = SinTable_Game();
    {
        const float turn = -orient->yaw;
        const X87Reg s = SinTable_At(sines, sines.scale * turn);
        const X87Reg c = SinTable_At(sines, (turn - kMinusHalfPi) * sines.scale);
        const X87Reg x = up.x, z = up.z;
        up.x = (float)(z * s + x * c);
        up.z = (float)(z * c - x * s);
    }
    {
        const float turn = -orient->pitch;
        const X87Reg s = SinTable_At(sines, sines.scale * turn);
        const X87Reg c = SinTable_At(sines, (turn - kMinusHalfPi) * sines.scale);
        const X87Reg y = up.y, z = up.z;
        up.y = (float)(y * c - z * s);
        up.z = (float)(z * c + y * s);
    }
    const double roll = Vec3_AngleZ(&up.x);
    orient->roll = (float)roll;

    if (*flags & 8) {
        auto bank = [&](const u8 *node) {
            const u32 i = (u32)(long long)field(node, 8);  // __ftol
            return (double)field(nodes + i * 0x38, 0x18);
        };
        const X87Reg ahead = X87Reg(bank(upTo)) * t;
        orient->roll = (float)((ahead + X87Reg(bank(upFrom)) * u) + roll);
    } else {
        const X87Reg k = X87Reg((double)*reinterpret_cast<const u32 *>(piece(upFrom))) + field(upFrom, 0xc);
        const X87Reg kUpTo = X87Reg((double)*reinterpret_cast<const u32 *>(piece(upTo))) + field(upTo, 0xc);
        const float mix = (float)(kUpTo * t + k * u);
        double whole;
        std::modf(k.v, &whole);  // CRT modf (0x67b280)
        X87Reg f = X87Reg(mix) - whole;
        if (!(f.v > 0)) f = 0.0;
        if (!(f.v < 1)) f = 1.0;
        orient->roll = (float)(((X87Reg(1.0f) - f) * field(piece(upFrom), 0xe4) + X87Reg(field(piece(upTo), 0xe4)) * f) +
                               orient->roll);
    }
    orient->roll2 = orient->roll;
}
}

extern "C" void __cdecl Coaster_FrameAt(u8 *coaster, float position, Vec3 *out, TrackOrientation *orient) {
    double whole;
    std::modf((double)position, &whole);  // CRT modf (0x67b280)
    const u32 i = (u32)(long long)whole;   // __ftol
    const u8 *nodes = *reinterpret_cast<const u8 *const *>(coaster + 0x9c);
    auto next = [](const u8 *node) { return *reinterpret_cast<const u8 *const *>(node + 0x18); };
    const u8 *from = *reinterpret_cast<const u8 *const *>(nodes + i * 0x38 + 0x2c);
    const u8 *upFrom = *reinterpret_cast<const u8 *const *>(nodes + i * 0x38 + 0x30);
    const u8 *to = next(from), *upTo = next(upFrom);
    while (!(field(to, 8) >= position)) {  // unordered counts as below
        from = to;
        to = next(to);
        upFrom = upTo;
        upTo = next(upTo);
    }
    const float t = (float)((X87Reg(position) - field(from, 8)) / (X87Reg(field(to, 8)) - field(from, 8)));
    TrackSpline_Frame(nodes, coaster + 0x3c, from, to, upFrom, upTo, t, out, orient);
}
