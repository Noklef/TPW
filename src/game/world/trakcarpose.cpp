#include "trakcarpose.h"

#include <cstdint>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "../model/heightat.h"
#include "../model/scratch.h"
#include "../render/particles.h"
#include "../terrain/land.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TrakPoseEnv kPortEnv = {&Terrain_RideAt, &Model_BeginView, &View_FloorAt, &Particle_Spawn};
const TrakPoseEnv *g_env = &kPortEnv;

// One wheel. A car's four are in a row; car, angle, x and y are set on its first only.
struct Wheel {
    u8 *car;
    u8 *cell;
    u32 angle;
    float x, y;
    float wheelX, wheelZ;
    float height;
};

const float kScale = 0x1.aaaaaap-9f;     // (0x700edc: 1 / 307.2)
const float kHalfPi = 0x1.921fb6p+0f;    // (0x700ef4)
const float kTwoPi = 0x1.921fb6p+2f;     // (0x700eec)

X87Reg fild(s32 n) { return X87Reg((double)n); }
s32 ftol(X87Reg x) { return (s32)(long long)x.v; }
s32 sine(u32 i) { return global<s32>(0x877358 + 4 * (i & 0x1ff)); }
u8 *model_of(u32 index) { return at<u8 *>(at<u8 *>(at<u8 *>(global<u8 *>(0x7a4610 + 4 * index), 8), 4), 0x78); }
float *matrix_of(u8 *model) { return reinterpret_cast<float *>(model + 0x10); }

void set_wheel(Wheel &w, const u8 *car, float x, float z) {
    w.wheelX = x;
    w.wheelZ = z;
    w.height = at<float>(car, 0xa0);
    w.cell = g_env->rideAt((u32)ftol(X87Reg(x) * 0.1f), (u32)ftol(X87Reg(z) * 0.1f));
}

void place_wheels(Wheel *w, u8 *car, const u8 *ride, float a) {
    const X87Reg back(-(double)a);
    const float x = (float)((fild(at<s32>(car, 0x34)) - fild(at<s32>(car, 0x3c)) * back) * kScale);
    const float y = (float)((fild(at<s32>(car, 0x38)) - fild(at<s32>(car, 0x40)) * back) * kScale);
    const u32 angle = ((u32)at<s32>(car, 0x54) - (u32)ftol(fild(at<s32>(car, 0x5c)) * back)) & 0x1ff;
    w->car = car;
    w->angle = angle;
    w->x = x;
    w->y = y;
    const X87Reg length = fild(at<s32>(ride, 8));
    const X87Reg alongX = fild((s32)((u32)sine(angle) << 2)) * length * 0x1p-10f * kScale;
    const X87Reg alongY = fild((s32)((u32)sine(angle + 0x80) << 2)) * length * 0x1p-10f * kScale;
    const float s = (float)alongX, c = (float)alongY;
    set_wheel(w[0], car, (float)(alongX + x), (float)(alongY + y));
    set_wheel(w[1], car, (float)(X87Reg(x) - s), (float)(X87Reg(y) - c));
    set_wheel(w[2], car, (float)(X87Reg(x) - c), (float)(X87Reg(s) + y));
    set_wheel(w[3], car, (float)(X87Reg(c) + x), (float)(X87Reg(y) - s));
}

// Each ride under the wheels in turn: its view current, the floor under its wheels.
void find_floors(Wheel *wheels, s32 count) {
    s32 next = count > 0 ? 0 : -1;
    while (next != -1) {
        const s32 first = next;
        next = -1;
        const u8 *const cell = wheels[first].cell;
        if (cell) g_env->beginView(at<const u8 *>(cell, 8));
        for (s32 j = first; j < count; j++) {
            Wheel &w = wheels[j];
            if (!w.cell) continue;
            if (w.cell != cell) {
                if (next == -1) next = j;
                continue;
            }
            g_env->floorAt(w.wheelX, w.wheelZ, &w.height);
            if (at<u32>(&w.height, 0) == 0xc2c80000) w.height = at<float>(wheels[j & ~3].car, 0xa0);
            w.cell = nullptr;
        }
    }
}

// The sine at +0x90 times step, ahead of its value a step back by the fraction a.
X87Reg sway(const u8 *car, u32 step, float a) {
    const u32 t = at<u32>(car, 0x90) * step;
    const X87Reg now = fild(sine(t));
    return now - X87Reg(-((now - fild(sine(t - step))) * a).v);
}

void pose_car(Wheel *w, const u8 *ride, float a) {
    u8 *const car = w->car;
    const float yaw = (float)(fild((s32)((w->angle - 0x100) & 0x1ff)) * 0x1p-9f * kTwoPi);
    u32 flags = at<u32>(car, 0);
    if (flags & 0x1000000) {
        w[0].height = (float)(sway(car, 6, a) * 0x1.47ae14p-10f + w[0].height);
        w[1].height = (float)(sway(car, 4, a) * 0x1.47ae14p-10f + w[1].height);
        w[2].height = (float)(sway(car, 3, a) * 0x1.47ae14p-10f + w[2].height);
        w[3].height = (float)(sway(car, 5, a) * 0x1.47ae14p-10f + w[3].height);
    }
    const X87Reg rise = fild((s32)((u32)at<s32>(ride, 8) << 1)) * kScale;
    const float pitch = (float)X87_Atan2(rise.v, (X87Reg(w[1].height) - w[0].height).v);
    const float roll = (float)X87_Atan2(rise.v, (X87Reg(w[3].height) - w[2].height).v);
    float tilt, height;
    if (flags & 0x800) {
        tilt = kHalfPi;
        height = (float)((X87Reg(w[1].height) + w[0].height) * 0.5f);
    } else {
        tilt = roll;
        height = (float)((X87Reg(w[3].height) + w[2].height + w[1].height + w[0].height) * 0.25f);
    }
    at<float>(car, 0xa0) = height;
    if (flags & 0x400) height = (float)(sway(car, 6, a) * 0x1.b4e81cp-10f + height - (-1.6f));
    if (flags & 0x1000) {
        const X87Reg change = fild(at<s32>(car, 0x5c)) - fild(at<s32>(car, 0x60)) * X87Reg(-(double)a);
        tilt = (float)(tilt - change * 0x1.3b13b2p-4f * -0x1.921fb6p+0f);
    }

    u8 *const model = model_of(at<u32>(car, 8));
    Matrix_ResetRotation(matrix_of(model));
    at<float>(model, 0x44) = height;
    at<float>(model, 0x40) = w->x;
    at<float>(model, 0x48) = w->y;
    const float bank = (float)(X87Reg(pitch) - kHalfPi);
    float angles[3] = {(float)(X87Reg(tilt) - kHalfPi), bank, yaw};
    Matrix_RotateEulerFast(matrix_of(model), 0xe, angles);

    flags = at<u32>(car, 0);
    if (flags & 0x4000000) {
        at<u32>(car, 0) = flags & ~0x4000000u;
        g_env->spawn(0xf, ftol(X87Reg(w->x) * 1024.0f), ftol(X87Reg(height) * 1024.0f), ftol(X87Reg(w->y) * 1024.0f));
    }
    if (at<u32>(car, 0) & 0x2000) {
        u8 *const second = model_of(at<u32>(car, 0xc));
        Matrix_ResetRotation(matrix_of(second));
        at<float>(second, 0x40) = w->x;
        at<float>(second, 0x44) = (float)((X87Reg(w[3].height) + w[2].height + w[1].height + w[0].height) * 0.25f);
        at<float>(second, 0x48) = w->y;
        angles[0] = (float)(X87Reg(roll) - kHalfPi);
        angles[1] = bank;
        angles[2] = yaw;
        Matrix_RotateEulerFast(matrix_of(second), 0xe, angles);
    }
    if (at<u32>(car, 0) & 0x2000000) {
        u8 *const second = model_of(at<u32>(car, 0xc));
        Matrix_ResetRotation(matrix_of(second));
        const u32 reach = at<u32>(car, 0x4c);
        const float divisor = global<float>(0x764cd8);
        w->x = (float)(w->x - fild((s32)(((u32)sine(w->angle) * reach) << 2)) / divisor);
        w->y = (float)(w->y - fild((s32)(((u32)sine(w->angle + 0x80) * reach) << 2)) / divisor);
        at<float>(second, 0x40) = w->x;
        at<float>(second, 0x44) = (float)(X87Reg(height) - (-0.7f));
        at<float>(second, 0x48) = w->y;
        angles[0] = 0.0f;
        angles[1] = 0.0f;
        angles[2] = yaw;
        Matrix_RotateEulerFast(matrix_of(second), 0xe, angles);
    }
}

}  // namespace

void TrakPose_SetEnv(const TrakPoseEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" void __cdecl TrakRide_PoseCars(u32 handle, float a) {
    if (!(a <= 1.0f)) a = 1.0f;
    else if (a < -1.0f) a = -1.0f;
    const bool loaded = global<u32>(0x877b58) == 0x4a454647;
    const u8 *const ride = loaded ? global<u8 *>(0x877b60) + (handle & 0xff) * 0xd0 : nullptr;
    if (handle != (loaded ? (handle & 0xff) | at<u32>(ride, 0) << 8 : 0)) return;
    u8 *car = global<u8 *>(0x877b68);
    if (!car) return;
    Wheel wheels[0x100];
    s32 count = 0;
    for (u32 left = global<u32>(0x877b6c); left; car += 0xac) {
        if (!(car[0] & 1)) continue;
        left--;
        if (at<const u8 *>(car, 0x9c) != ride) continue;
        place_wheels(wheels + count, car, ride, a);
        count += 4;
    }
    find_floors(wheels, count);
    for (s32 i = 0; i < count; i += 4) pose_car(wheels + i, ride, a);
}
