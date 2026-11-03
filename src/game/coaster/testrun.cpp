#include "testrun.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../math/matrix.h"
#include "../math/sintable.h"
#include "../math/x87.h"
#include "../model/holder.h"
#include "carsound.h"
#include "cars.h"
#include "rating.h"
#include "setup.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &g(u32 address) { return *reinterpret_cast<T *>(address); }

const TestRunEnv kPortEnv = {&CoasterRating_Reset, &CoasterRating_Step, &CoasterRating_Finish,  &Coaster_SetTrainCount, &CoasterCar_Wake,
                             &Obj_RemoveFromRect,  &Holder_Sleep,       &CoasterCar_StopSounds, &Coaster_SpaceCars};
const TestRunEnv *g_env = &kPortEnv;

const float kOne = 1.0f;                   // 0x6fe020 (and 0x74ce34)
const float kZero = 0.0f;                  // 0x6fe024
const float kPi = 3.14159274f;             // 0x6fe028
const float kMinusPi = -3.14159274f;       // 0x6fe02c
const float kThousandth = 0.00100000005f;  // 0x6fe030
const float kClimb = 0.0174499992f;        // 0x6fe038
const float kTenth = 0.100000001f;         // 0x6fe03c
const float kMinusHalfPi = -1.57079637f;   // 0x6fe040
const float kMinusTwoPi = -6.28318548f;    // 0x6fe058
const double kMinusThreePi = X87_FromBits(0xc022d97c88000000ull);   // 0x6fe048
const double kTwoPi = X87_FromBits(0x401921fb60000000ull);          // 0x6fe050

X87Reg neg(X87Reg x) { return X87Reg(-x.v); }   // fchs

// The sine table's entry for an angle already scaled (fistp of the float).
X87Reg sine(float scaled) { return X87Reg(SinTable_At(SinTable_Game(), scaled)); }
const float &scale() { return g<float>(0x007b095c); }
// scale * a, and (a - -pi/2) * scale (the cosine's), each stored to a float before the fistp.
float scaled(float a) { return (float)(X87Reg(scale()) * a); }
float scaled_cos(float a) { return (float)((X87Reg(a) - kMinusHalfPi) * scale()); }
// The turn by the node's angle at +off: -angle (fld; fchs; fstp), its sine and cosine.
void angle(const u8 *node, u32 off, X87Reg &s, X87Reg &c) {
    const float a = (float)neg(X87Reg(at<float>(node, off)));
    s = sine(scaled(a));
    c = sine(scaled_cos(a));
}

// a = s b + c a, b = c b - a s (about y: x and z).
void turn_y(float &a, float &b, X87Reg s, X87Reg c) {
    const X87Reg old = X87Reg(a);
    const X87Reg sb = s * b;
    a = (float)(sb + c * old);
    b = (float)(c * b - old * s);
}

// a = c a - s b, b = c b + a s (about x: y and z; about z: x and y).
void turn(float &a, float &b, X87Reg s, X87Reg c) {
    const X87Reg old = X87Reg(a);
    const X87Reg ca = c * a;
    a = (float)(ca - s * b);
    b = (float)(c * b + old * s);
}

// fmod (x - -3pi, 2pi) - pi (_CIfmod; fsub pi).
X87Reg wrap(X87Reg x) { return X87Reg(std::fmod((x - X87Reg(kMinusThreePi)).v, kTwoPi)) - kPi; }

// The first loop's speeds and forces; returns the run's time, with the top speed and height.
float speeds(u8 *coaster, float least, float &topSpeed, float &maxHeight) {
    u8 *const type = at<u8 *>(coaster, 4);
    const float cap = at<float>(coaster, 0x34);
    const float step = at<float>(coaster, 0xac);
    u8 *node = at<u8 *>(coaster, 0x9c) + 0x38;
    float v = least, time = 0.0f;
    for (u32 i = 1; i < at<u32>(coaster, 0xa4) - 1; ++i, node += 0x38) {
        const u8 *const prev = node - 0x38, *const next = node + 0x38;
        const float height = at<float>(node, 8);
        u8 *const kind = at<u8 *>(at<u8 *>(node, 0x2c), 0x10);
        const X87Reg dx1 = X87Reg(at<float>(prev, 4)) - at<float>(node, 4);
        const float dy1 = (float)(X87Reg(at<float>(prev, 8)) - height);
        const float dz1 = (float)(X87Reg(at<float>(prev, 0xc)) - at<float>(node, 0xc));
        const X87Reg dx2 = X87Reg(at<float>(next, 4)) - at<float>(node, 4);
        const float dy2 = (float)(X87Reg(at<float>(next, 8)) - height);
        const float dz2 = (float)(X87Reg(at<float>(next, 0xc)) - at<float>(node, 0xc));
        if (at<u32>(type, 0) & 0x100) {
            const X87Reg length = X87_Sqrt(dx2 * dx2 + X87Reg(dy2) * dy2 + X87Reg(dz2) * dz2);
            if ((dy2 / length).v > (double)kClimb)
                at<u32>(node, 0) |= 8;
            else
                at<u32>(node, 0) &= ~8u;
        }
        const float fx = (float)neg((dx1 + dx2) / step * kOne);
        const float fy = (float)neg((X87Reg(dy1) + dy2) / step * kOne);
        const float fz = (float)neg((X87Reg(dz1) + dz2) / step * kOne);
        const X87Reg above = X87Reg(at<float>(next, 8));
        X87Reg s = (height - above) * at<float>(type, above.v > (double)height ? 0x28 : 0x2c) * at<float>(type, 0x24) + v;
        if (!(s.v > (double)least)) s = X87Reg(least);
        v = (float)s;
        if (at<u8>(node, 0) & 8) v = (float)(X87Reg(at<float>(type, 0x54)) * s);
        if (!(v > least)) v = least;
        if (!(v < least || v > least)) at<u32>(kind, 8) |= 0x800;
        if (v >= cap) v = cap;   // (fcom: a NaN keeps v)
        at<float>(node, 0x1c) = v;
        float *const force = &at<float>(node, 0x20);
        force[0] = fx;
        force[1] = fy;
        force[2] = fz;
        for (int k = 0; k < 3; ++k) force[k] = (float)(X87Reg(force[k]) * v * kTenth);
        g_env->ratingStep(coaster, i, at<u32>(at<u8 *>(at<u8 *>(node, 0x2c), 0x10), 0), v, at<float>(node, 0x18), force, time);
        if (!(topSpeed > v)) topSpeed = v;
        if (!(maxHeight > height)) maxHeight = height;
        time = (float)(X87Reg(step) / v + time);
    }
    return time;
}

// The type's flag 8: the nodes' banks.
void banks(u8 *coaster) {
    u8 *const type = at<u8 *>(coaster, 4);
    const float ux = at<float>(type, 0x60), uy = at<float>(type, 0x64), uz = at<float>(type, 0x68);
    float last = kPi;
    u8 *node = at<u8 *>(coaster, 0x9c);
    for (u32 j = 0; j < at<u32>(coaster, 0xa4); ++j, node += 0x38) {
        float w[3] = {(float)(X87Reg(at<float>(node, 0x20)) + ux), (float)(X87Reg(at<float>(node, 0x24)) + uy),
                      (float)(X87Reg(at<float>(node, 0x28)) + uz)};
        X87Reg s = 0.0, c = 0.0;
        angle(node, 0x10, s, c);
        turn_y(w[0], w[2], s, c);
        angle(node, 0x14, s, c);
        turn(w[1], w[2], s, c);
        angle(node, 0x18, s, c);
        turn(w[0], w[1], s, c);
        const float d = (float)wrap(X87Reg(Vec3_AngleZ(w)) - last);
        const X87Reg length =
            X87_Sqrt(X87Reg(w[0]) * X87Reg(w[0]) + X87Reg(w[1]) * w[1] + X87Reg(w[2]) * w[2]) * at<float>(at<u8 *>(coaster, 4), 0x6c);
        const float most = length.v > (double)kOne ? kOne : (float)length;
        const X87Reg e = wrap(X87Reg(d) * most + last);
        const X87Reg bank = neg(X87Reg(kPi) - e);
        at<float>(node, 0x18) = (float)bank;
        if (!(bank.v >= (double)kMinusPi)) at<float>(node, 0x18) = (float)(bank - kMinusTwoPi);
        const X87Reg index = X87Reg((double)(s32)j);
        const float from = at<float>(coaster, 0x134);
        if (index.v > (double)from) {
            X87Reg t = (index - from) / (X87Reg(at<float>(coaster, 0xa8)) - from);
            if (t.v >= (double)kOne) t = X87Reg(kOne);
            if (!(t.v > (double)kZero)) t = X87Reg(kZero);
            at<float>(node, 0x18) = (float)((X87Reg(kOne) - t) * at<float>(node, 0x18));
        }
        last = (float)e;
    }
}

// The trains the run's time allows: woken, or put to sleep.
void trains(u8 *coaster, float time) {
    u8 *const type = at<u8 *>(coaster, 4);
    const s32 fit = (s32)(long long)(time / X87Reg(X87_MulInt(at<u32>(type, 0x2ec), kThousandth))).v;
    u32 count = fit < 1 ? 1 : (u32)fit;
    if (!(count < at<u32>(type, 0x2e8))) count = at<u32>(type, 0x2e8);
    g_env->setTrainCount(coaster, count);
    u8 *const *const strips = reinterpret_cast<u8 *const *>(0x007a4610);
    u8 *train = at<u8 *>(coaster, 0x128);
    for (u32 t = 0; t < at<u32>(at<u8 *>(coaster, 4), 0x2e8); ++t, train += 0x80) {
        if (t < at<u32>(coaster, 0xf4)) {
            if (!(at<u32>(train, 0) & 1)) {
                at<u32>(train, 0) |= 5;
                if (at<u32>(coaster, 0) & 0x800) g_env->wake(coaster, train);
            }
        } else if (at<u32>(train, 0) & 1) {
            at<u32>(train, 0) &= ~1u;
            if (at<u32>(coaster, 0) & 0x800) {
                u8 *car = at<u8 *>(train, 0x28) + at<u32>(train, 8) * 0x60;
                for (u32 n = at<u32>(train, 0xc); n != 0; --n, car += 0x60) {
                    at<u32>(car, 0) &= ~1u;
                    if (const u32 index = at<u32>(car, 0xc)) {
                        u8 *const strip = strips[index];
                        if (at<u32>(strip, 4) & 0x4000)
                            g_env->removeFromRect(at<u8 *>(strip, 8), at<u32>(strip, 0xc0), at<u32>(strip, 0xc4));
                        else
                            g_env->holderSleep(at<u8 *>(strip, 8));
                    }
                }
                g_env->stopSounds(coaster, train);
                at<u32>(train, 0) &= ~2u;
            }
        }
    }
}

// Each running train's cars along the track, its ends and its average speed.
void places(u8 *coaster) {
    u8 *train = at<u8 *>(coaster, 0x128);
    for (u32 t = 0; t < at<u32>(coaster, 0xf4); ++t, train += 0x80) {
        u8 *const cars = at<u8 *>(train, 0x28);
        X87Reg p = X87Reg(at<float>(train, 0x30));
        u8 *car = cars + at<u32>(train, 0xc) * 0x60 - 0x60;
        for (u32 k = 0; k < at<u32>(train, 0xc); ++k, car -= 0x60) {
            at<float>(car, 0x40) = (float)p;
            if (k < at<u32>(train, 0xc) - 1)
                p = p + (X87Reg(at<float>(at<u8 *>(car, 4), 0x24)) + at<float>(at<u8 *>(car - 0x60, 4), 0x28)) / at<float>(coaster, 0xac);
        }
        const X87Reg rear = X87Reg(at<float>(cars + at<u32>(train, 0x2c) * 0x60, 0x40));
        at<float>(train, 0x74) = (float)rear;
        u8 *const first = cars + at<u32>(train, 8) * 0x60;
        u8 *const end = first + at<u32>(train, 0xc) * 0x60 - 0x60;
        at<float>(train, 0x20) = (float)(X87Reg(at<float>(first, 0x38)) / at<float>(coaster, 0xac) + rear +
                                         X87Reg(at<float>(at<u8 *>(first, 4), 0x24)) / at<float>(coaster, 0xac));
        at<float>(train, 0x24) = (float)(X87Reg(at<float>(end, 0x38)) / at<float>(coaster, 0xac) + at<float>(train, 0x74) -
                                         X87Reg(at<float>(at<u8 *>(end, 4), 0x28)) / at<float>(coaster, 0xac));
        float sum = 0.0f;
        const u32 n = at<u32>(train, 0xc);
        const u8 *pos = at<u8 *>(train, 0x28) + 0x40;
        for (u32 k = n; k != 0; --k, pos += 0x60) {
            double whole;
            const X87Reg frac = X87Reg(std::modf((double)at<float>(pos, 0), &whole));
            const u8 *const node = at<u8 *>(coaster, 0x9c) + (u32)(s32)(long long)whole * 0x38;
            sum = (float)(X87Reg(at<float>(node, 0x1c)) * (X87Reg(kOne) - frac) + X87Reg(at<float>(node, 0x54)) * frac + sum);
        }
        at<float>(train, 0x78) = (float)(sum / X87Reg((double)at<u32>(train, 0xc)));
        at<u32>(train, 0) &= ~4u;
    }
}

}  // namespace

void TestRun_SetEnv(const TestRunEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

void __cdecl Coaster_TestRun(u8 *coaster) {
    u8 *const type = at<u8 *>(coaster, 4);
    const float least = X87_Through(at<float>(type, 0x44));
    float topSpeed = 0.0f, maxHeight = 0.0f;
    g_env->ratingReset(coaster);
    const float time = speeds(coaster, least, topSpeed, maxHeight);
    u8 *const nodes = at<u8 *>(coaster, 0x9c);
    at<u32>(nodes, 0x20) = at<u32>(nodes, 0x24) = at<u32>(nodes, 0x28) = 0;
    at<float>(nodes, 0x1c) = least;
    u8 *const last = at<u8 *>(coaster, 0x9c) + at<u32>(coaster, 0xa4) * 0x38 - 0x38;
    std::memcpy(last + 0x1c, last - 0x38 + 0x1c, 0x10);
    const u32 count = at<u32>(coaster, 0xa4);
    const u8 *const lead = at<u8 *>(coaster, 0x9c) + at<u32>(at<u8 *>(coaster, 0x58), 0xfc) * 0x38 - 0x38;
    X87Reg from = X87Reg((double)(count - 1)) - X87Reg(at<float>(lead, 0x1c)) / at<float>(coaster, 0xac);
    if (!(from.v > (double)kZero)) from = X87Reg(kZero);
    at<float>(coaster, 0x134) = (float)from;
    if (at<u8>(type, 0) & 8) banks(coaster);
    g_env->ratingFinish(coaster, maxHeight, topSpeed, time);
    if (!(at<u32>(coaster, 0) & 0x1000)) return;
    if (!(at<u32>(coaster, 0) & 0x4000)) {
        trains(coaster, time);
        g_env->spaceCars(coaster);
        places(coaster);
    }
    at<u32>(coaster, 0) &= ~0x1000u;
}
}
