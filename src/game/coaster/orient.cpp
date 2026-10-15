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
}
