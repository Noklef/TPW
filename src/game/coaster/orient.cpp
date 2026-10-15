#include "orient.h"

#include <cstring>

#include "../math/matrix.h"
#include "../math/sintable.h"
#include "../math/x87.h"

namespace {

const float kPi = 3.14159274f;        // 0x6fe120
const float kTwoPi = 6.28318548f;     // 0x6fe124
const float kMinusHalfPi = -1.57079637f;  // 0x6fe138
const float kMinusPi = -3.14159274f;  // 0x6fe13c

u32 bits(float f) {
    u32 b;
    std::memcpy(&b, &f, 4);
    return b;
}

float value(u32 b) {
    float f;
    std::memcpy(&f, &b, 4);
    return f;
}

}  // namespace

extern "C" {

void __cdecl TrackOrientation_Set(Vec3 direction, u32, TrackOrientation *out, const float *yaw) {
    u32 yawBits;
    if (yaw)
        std::memcpy(&yawBits, yaw, 4);
    else
        yawBits = bits((float)Vec3_AngleY(&direction.x));
    std::memcpy(&out->yaw, &yawBits, 4);

    const float turn = -value(X87_FloatThrough(yawBits));
    const SinTable sines = SinTable_Game();
    const X87Reg s = SinTable_At(sines, sines.scale * turn);
    const X87Reg c = SinTable_At(sines, (turn - kMinusHalfPi) * sines.scale);
    const X87Reg x = direction.x, z = direction.z;
    direction.x = (float)(s * z + c * x);
    direction.z = (float)(c * z - x * s);
    const float pitch = (float)Vec3_AngleX(&direction.x);
    out->pitch = pitch;
    std::memcpy(&out->rawYaw, &yawBits, 4);

    if (out->flags & 1) {
        const X87Reg turned = X87Reg(value(X87_FloatThrough(yawBits))) - kMinusPi;
        out->yaw = (float)turned;
        const float flipped = (float)(X87Reg(kPi) - pitch);
        out->pitch = flipped;
        if (turned.v > kPi) out->yaw = (float)(turned - kTwoPi);
        if (flipped > kPi) out->pitch = (float)(X87Reg(flipped) - kTwoPi);
    }
    std::memcpy(&out->yaw2, &out->yaw, 4);
    std::memcpy(&out->pitch2, &out->pitch, 4);
}

void __cdecl TrackOrientation_Init(u32, const Vec3 *direction, u32 unused, TrackOrientation *out, u32,
                                   const float *yaw) {
    std::memset(out, 0, 0x1c);
    const u32 max = 0x7f7fffff;  // FLT_MAX
    std::memcpy(&out->rawYaw, &max, 4);
    TrackOrientation_Set(*direction, unused, out, yaw);
}

void __cdecl Track_RefreshOrientations() {
    const u8 *track = *reinterpret_cast<u8 *const *>(0x00790fe4);
    for (u8 *piece = *reinterpret_cast<u8 *const *>(track + 0x18); piece; piece = *reinterpret_cast<u8 **>(piece + 0x64)) {
        const u32 *list = *reinterpret_cast<const u32 *const *>(piece + 0x90);
        const u8 *node = reinterpret_cast<const u8 *>(list[1]);
        for (u32 i = 0; i < **reinterpret_cast<const u32 *const *>(piece + 0x90); ++i, node += 0x58) {
            const float at = (float)(X87Reg(*reinterpret_cast<const float *>(node + 0xc)) +
                                     *reinterpret_cast<const float *>(piece + 0x6c));
            u32 bits;
            std::memcpy(&bits, &at, 4);
            TrackOrientation dropped;
            TrackOrientation_Init((u32)(uintptr_t)track, reinterpret_cast<const Vec3 *>(node + 0x28), bits, &dropped, 1,
                                  nullptr);
        }
        u32 &flags = *reinterpret_cast<u32 *>(piece + 8);
        if (!(flags & 0x1000)) flags &= ~0x800u;
    }
}

void __cdecl RideObject_Orient(u8 *object, const float *angles, float *v) {
    const u8 *ride = reinterpret_cast<const u8 *const *>(0x007a4610)[*reinterpret_cast<const u32 *>(object + 0xc)];
    const u8 *holder = *reinterpret_cast<const u8 *const *>(*reinterpret_cast<const u8 *const *>(ride + 8) + 4);
    float *m = reinterpret_cast<float *>(*reinterpret_cast<u8 *const *>(holder + 0x78) + 0x10);
    Matrix_Identity(m);
    const SinTable sines = SinTable_Game();
    const auto sine = [&](float a) { return SinTable_At(sines, (float)(X87Reg(sines.scale) * a)); };
    const auto cosine = [&](float a) { return SinTable_At(sines, (float)((X87Reg(a) - kMinusHalfPi) * sines.scale)); };
    Matrix_RotateZ(m, cosine(angles[3]), sine(angles[3]));
    Matrix_RotateX(m, cosine(angles[2]), sine(angles[2]));
    Matrix_RotateY(m, cosine(angles[1]), sine(angles[1]));
    if (v) {
        const auto turn = [&](float a, int i, int j) {  // about z (0, 1) and x (1, 2)
            const X87Reg s = sine(a), c = cosine(a), first = v[i];
            v[i] = (float)(first * c - s * v[j]);
            v[j] = (float)(c * v[j] + first * s);
        };
        turn(angles[3], 0, 1);
        turn(angles[2], 1, 2);
        const X87Reg s = sine(angles[1]), c = cosine(angles[1]), first = v[0];  // about y
        v[0] = (float)(s * v[2] + first * c);
        v[2] = (float)(c * v[2] - first * s);
    }
    std::memcpy(object + 0x10, m + 8, 12);
}
}
