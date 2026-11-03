#include "trainrun.h"

#include <emmintrin.h>

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "../motion/animator.h"
#include "bendmesh.h"
#include "carsound.h"
#include "cars.h"
#include "orient.h"
#include "spline.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const float kOne = 1.0f;          // 0x6fe020
const float kZero = 0.0f;         // 0x6fe024
const float kThousandth = 0.00100000005f;   // 0x6fe030
const float kHalf = 0.5f;         // 0x6fe064
const float kMinusHalf = -0.5f;   // 0x6fe068
const float kTwo = 2.0f;          // 0x6fe06c
const float kDrag = 0.949999988f;     // 0x6fe070
const float kStopped = 0.00999999978f;   // 0x6fe074
const float kFast = 0.300000012f;     // 0x6fe078
const float kThousand = 1000.0f;  // 0x6fe07c
const u32 kNode = 0x38;
const u32 kCar = 0x60;
const u32 kTrain = 0x80;

void __cdecl PortFrameAt(u8 *coaster, float position, float *out, u8 *orient) {
    Coaster_FrameAt(coaster, position, reinterpret_cast<Vec3 *>(out), reinterpret_cast<TrackOrientation *>(orient));
}
const TrainRunEnv kPortEnv = {&Motion_Time, &CoasterCar_Bump, &PortFrameAt, &RideObject_Orient, &Layer_SetSpeed, &Coaster_BendMesh, &CoasterCar_SetSoundMode};
const TrainRunEnv *g_env = &kPortEnv;

u32 Ftol(double x) { return (u32)(s32)(long long)x; }

// The track place x: the two nodes around it and their weights. (A negative x gives an index off the table, sic.)
struct NodePair {
    X87Reg f, g;
    const u8 *a, *b;
};
NodePair Between(const u8 *c, double x) {
    double whole;
    const double f = std::modf(x, &whole);
    const u32 index = Ftol(whole);
    const u8 *nodes = at<u8 *>(c, 0x9c);
    return {f, X87Reg(kOne) - X87Reg(f), nodes + index * kNode, nodes + (index + 1) * kNode};
}
float Mix(const NodePair &n, u32 offset) { return (float)((X87Reg(at<float>(n.b, offset)) * n.f) + (X87Reg(at<float>(n.a, offset)) * n.g)); }

// The place of a car (+0x38 over the spacing, + the train's place).
X87Reg CarPlace(const u8 *c, const u8 *train, const u8 *car) { return (X87Reg(at<float>(car, 0x38)) / at<float>(c, 0xac)) + at<float>(train, 0x74); }

// x below zero (or NaN): x + length, else fmod (x, length).
X87Reg Wrapped(float x, float length) {
    if (!(x >= 0.0f)) return X87Reg(x) + length;
    return std::fmod((double)x, (double)length);
}

void SetNodePoints(const u8 *c, const u8 *train) {
    u8 *car = at<u8 *>(train, 0x28);
    for (u32 k = 0; k < at<u32>(train, 0xc); ++k, car += kCar) {
        if (!(at<u8>(car, 0) & 1)) continue;
        const X87Reg v = CarPlace(c, train, car);
        const float t = (float)v;
        const NodePair n = Between(c, Wrapped(t, at<float>(c, 0xa8)).v);
        at<float>(car, 0x48) = Mix(n, 4);
        at<float>(car, 0x4c) = Mix(n, 8);
        at<float>(car, 0x50) = Mix(n, 0xc);
    }
}

// The cars' points half a node on and their speeds; the mean, whether any car is on a chain and how many are past +0x134.
struct Speeds {
    float sum;
    u32 chain, past;
};
Speeds CarSpeeds(const u8 *c, const u8 *type, const u8 *train, float length, float limit, float bottom) {
    Speeds s = {0.0f, 0, 0};
    u8 *car = at<u8 *>(train, 0x28);
    for (u32 k = 0; k < at<u32>(train, 0xc); ++k, car += kCar) {
        if (!(at<u8>(car, 0) & 1)) continue;
        float t = (float)CarPlace(c, train, car);
        if (!(t >= 0.0f)) t = (float)(X87Reg(length) + t);
        const float behind = (float)(X87Reg(t) - kMinusHalf);
        const NodePair n = Between(c, Wrapped(behind, length).v);
        at<float>(car, 0x54) = Mix(n, 4);
        at<float>(car, 0x58) = Mix(n, 8);
        at<float>(car, 0x5c) = Mix(n, 0xc);
        const u8 *kind = at<u8 *>(at<u8 *>(n.a, 0x2c), 0x10);
        if (t > at<float>(c, 0x134)) ++s.past;
        const float from = at<float>(car, 0x4c), to = at<float>(car, 0x58);
        const X87Reg scaled = (X87Reg(from) - X87Reg(to)) * X87Reg(at<float>(type, 0x24));
        X87Reg x = (scaled * X87Reg(at<float>(type, to > from ? 0x28 : 0x2c))) + X87Reg(at<float>(train, 0x78));
        if (at<u32>(kind, 8) & 0x800) {
            if (!(x.v > limit)) x = X87Reg(limit);
            s.sum = (float)(x + s.sum);
            s.chain = 1;
            at<u32>(car, 0) |= 2;
        } else {
            if (!(x.v > bottom)) x = X87Reg(bottom);
            s.sum = (float)(x + s.sum);
            at<u32>(car, 0) &= ~2u;
        }
    }
    return s;
}

// The list: the head off it, or it put back in by the time it has left.
void TakeOff(u8 **sorted, u32 left) {
    for (u32 j = 0; j < left; ++j) sorted[j] = sorted[j + 1];
    sorted[left] = nullptr;
}
void PutBack(u8 **sorted, u32 left, u8 *train) {
    for (u32 j = 1; j < left; ++j) {
        u8 *other = sorted[j];
        if (!(at<float>(other, 0x7c) > at<float>(train, 0x7c))) {
            sorted[j - 1] = train;
            return;
        }
        sorted[j - 1] = other;
    }
    sorted[left - 1] = train;
}

void StepTrain(u8 *c, const u8 *type, u8 **sorted, u32 &left, float cap, float length, float topA, float bottom, u32 &wBits) {
    u8 *train = sorted[0];
    const float v = at<float>(train, 0x74);
    const float pos = !(v >= length) ? X87_Through(v) : length;
    u8 *cars = at<u8 *>(train, 0x28);
    const u8 *lead = cars + at<u32>(train, 0x2c) * kCar;
    const u32 f0 = at<u32>(train, 0);
    const float limit = (f0 & 0x80) && (f0 & 0x20) ? (float)(X87Reg(topA) * kHalf) : topA;
    const Speeds s = CarSpeeds(c, type, train, length, limit, bottom);
    const float mean = (float)(X87Reg(s.sum) / X87Reg((double)at<u32>(train, 0xc)));
    const float dist = (float)Vec3_Distance(reinterpret_cast<const float *>(lead + 0x48), reinterpret_cast<const float *>(lead + 0x54));

    X87Reg a(0.0);
    if (s.past && !(at<u32>(train, 0) & 0x40)) {
        const u8 *mark = cars + at<u32>(train, 8) * kCar;
        X87Reg u = (CarPlace(c, train, mark) - at<float>(c, 0x134)) / (X87Reg(length) - at<float>(c, 0x134));
        if (!(u.v >= 0.0) || u.v > 1.0) u = X87Reg(kOne);
        const X87Reg square = u * u;
        const float w = (float)(square * square);
        std::memcpy(&wBits, &w, 4);
        at<u32>(train, 0) |= 0x10;
        a = (X87Reg(limit) * w) + ((X87Reg(kOne) - w) * mean);
    } else {
        const u32 index = Ftol(at<float>(lead, 0x40));
        a = at<u8>(at<u8 *>(c, 0x9c) + index * kNode, 0) & 8 ? X87Reg(at<float>(type, 0x54)) * mean : X87Reg(mean);
        at<u32>(train, 0) &= ~0x10u;
    }

    if (s.chain) {
        if (!(a.v > limit)) a = X87Reg(limit);
        at<u32>(train, 0) |= 8;
    } else {
        if (!(a.v > bottom)) a = X87Reg(bottom);
        at<u32>(train, 0) &= ~8u;
    }
    if (a.v >= cap) a = X87Reg(cap);

    const float tt = (float)(dist / ((a + at<float>(train, 0x78)) * kHalf));
    if (tt >= at<float>(train, 0x7c)) {
        const X87Reg r = X87Reg(at<float>(train, 0x7c)) / tt;
        const X87Reg a2 = (a * r) + ((X87Reg(kOne) - r) * at<float>(train, 0x78));
        at<float>(train, 0x74) = (float)(pos - ((((((a2 + at<float>(train, 0x78)) * at<float>(train, 0x7c)) * kHalf) / dist) * kMinusHalf)));
        at<float>(train, 0x78) = (float)(a2.v >= cap ? X87Reg(cap) : a2);
        --left;
        TakeOff(sorted, left);
    } else {
        at<float>(train, 0x78) = (float)a;
        at<float>(train, 0x7c) = (float)(X87Reg(at<float>(train, 0x7c)) - tt);
        at<float>(train, 0x74) = (float)(X87Reg(at<float>(train, 0x74)) - kMinusHalf);
        u8 *car = cars;
        for (u32 k = 0; k < at<u32>(train, 0xc); ++k, car += kCar) {
            at<u32>(car, 0x48) = at<u32>(car, 0x54);
            at<u32>(car, 0x4c) = at<u32>(car, 0x58);
            at<u32>(car, 0x50) = at<u32>(car, 0x5c);
        }
        PutBack(sorted, left, train);
    }

    const float m = !(at<float>(train, 0x78) >= kTwo) ? kDrag : kOne;
    const X87Reg x = X87Reg(m) * at<float>(train, 0x78);
    at<float>(train, 0x78) = !(x.v >= kStopped) ? 0.0f : (float)x;
    const u8 *mark = cars + at<u32>(train, 8) * kCar;
    at<float>(train, 0x20) = (float)(CarPlace(c, train, mark) + (X87Reg(at<float>(at<u8 *>(mark, 4), 0x24)) / at<float>(c, 0xac)));
    const u8 *last = mark + at<u32>(train, 0xc) * kCar - kCar;   // (sic: from the first, not from car 0)
    at<float>(train, 0x24) = (float)(CarPlace(c, train, last) - (X87Reg(at<float>(at<u8 *>(last, 4), 0x28)) / at<float>(c, 0xac)));
    g_env->bump(c, train);
    if (at<float>(train, 0x74) >= at<float>(c, 0xa8)) at<float>(train, 0x74) = (float)std::fmod((double)at<float>(train, 0x74), (double)length);
    if (at<float>(train, 0x74) > at<float>(c, 0xfc) && !(at<float>(train, 0x74) >= at<float>(c, 0x134))) at<u32>(train, 0) &= ~0x40u;
}

void PlaceCar(u8 *c, u8 *train, u8 *car, u32 k, float length) {
    const float p = (float)CarPlace(c, train, car);
    at<float>(car, 0x40) = (float)Wrapped(p, length);
    float out[3];
    u8 *orient = c + 0x78;
    g_env->frameAt(c, at<float>(car, 0x40), out, orient);
    const u32 ride = at<u32>(car, 0xc);
    const u8 *model = at<u8 *>(car, 4);
    if (ride) {
        const float lift = at<float>(model, 0x18);
        if (!(lift < 0.0f || lift > 0.0f)) {   // zero or NaN
            g_env->orient(car, reinterpret_cast<const float *>(orient), nullptr);
        } else {
            float offset[3] = {0.0f, lift, 0.0f};
            g_env->orient(car, reinterpret_cast<const float *>(orient), offset);
            out[0] = (float)(X87Reg(offset[0]) + out[0]);
            out[1] = (float)(X87Reg(offset[1]) + out[1]);
            out[2] = (float)(X87Reg(offset[2]) + out[2]);
        }
        u8 *holder = *reinterpret_cast<u8 **>((uintptr_t)(0x007a4610 + ride * 4));
        u8 *matrix = at<u8 *>(at<u8 *>(at<u8 *>(holder, 8), 4), 0x78);
        at<u32>(matrix, 0x40) = reinterpret_cast<const u32 *>(out)[0];
        at<float>(matrix, 0x44) = X87_Through(out[1]);
        at<float>(matrix, 0x48) = X87_Through(out[2]);
        if (!(at<u8>(model, 0) & 1)) g_env->setSpeed(holder, (float)(X87Reg(at<float>(model, 0x1c)) * X87Reg(at<float>(train, 0x78))), 0);
    }
    if (k == at<u32>(train, 0x2c)) std::memcpy(train + 0x48, out, 12);
    if (k == at<u32>(train, 8)) std::memcpy(train + 0x3c, out, 12);
    if (at<u8>(at<u8 *>(car, 4), 0) & 1) g_env->bend(c, car);
}

void SetSound(const u8 *c, u8 *train, u32 onChain, u32 wBits) {
    if (at<u32>(train, 0) & 0x10) {
        // sic: the fourth power as last computed, for whichever train; or the bits of the end of the train array when none has computed it.
        float w;
        std::memcpy(&w, &wBits, 4);
        const s32 n = _mm_cvtss_si32(_mm_set_ss(w));
        g_env->setSound(train, 8, (u32)(n + 1) * 50);
        return;
    }
    const float mix = at<float>(c, 0x2c);
    if (onChain && (mix < 0.0f || mix > 0.0f)) {
        // sic: a train type of +4 0 faults here.
        const u32 q = onChain * 100 / at<u32>(at<u8 *>(train, 4), 4);
        const bool fast = (X87Reg(at<float>(train, 0x78)) / at<float>(c, 0x34)).v > kFast;
        g_env->setSound(train, fast ? 4 : 2, q);
        return;
    }
    g_env->setSound(train, 0x10, 100);
}

void ClearSpacedFlag(const u8 *c, const u8 *type, u8 *train, u32 i, float spacing) {
    const u32 flags = at<u32>(train, 0);
    if (!(flags & 0x80)) return;
    const u8 *first = at<u8 *>(c, 0x128);
    const u32 next = ((u32)(((s32)(train - first) >> 7) + 1)) % at<u32>(c, 0xf4);
    u32 e = Ftol(at<float>(first + next * kTrain, 0x74));
    X87Reg acc(kZero);
    if (!((double)e >= at<float>(train, 0x74))) e = e + at<u32>(c, 0xa4) - 1;
    const u32 s = Ftol(at<float>(train, 0x74));
    if (s < e) {
        const X87Reg step = X87Reg(spacing) / at<float>(at<u8 *>(c, 0x9c) + (i % at<u32>(c, 0xa4)) * kNode, 0x1c);   // (sic: the train's number picks the node)
        for (u32 n = e - s; n; --n) acc = acc + step;
    }
    if (!((double)at<u32>(type, 0x2ec) > (acc * kThousand).v)) at<u32>(train, 0) = flags & ~0x80u;
}

}  // namespace

void TrainRun_SetEnv(const TrainRunEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" void __cdecl Coaster_RunTrains(u8 *c) {
    const u8 *type = at<u8 *>(c, 4);
    u8 **sorted = at<u8 **>(c, 0x12c);
    const u32 now = g_env->time(0);
    u8 *first = at<u8 *>(c, 0x128);
    at<u8 *>(c, 0x124) = first;
    const float topA = (float)(X87Reg(at<float>(type, 0x44)) * at<float>(c, 0x2c));
    const float cap = at<float>(c, 0x34);
    const float length = at<float>(c, 0xa8);
    const float spacing = at<float>(c, 0xac);
    u32 left = at<u32>(c, 0xf4);
    const float bottom = (float)(X87Reg(at<float>(type, 0x48)) * at<float>(c, 0x2c));
    if (!(at<u32>(c, 0) & 0x10)) {
        at<u32>(c, 0) |= 0x10;
        at<u32>(c, 0x120) = now;
    }
    const float elapsed = (float)X87_MulInt(now - at<u32>(c, 0x120), kThousandth);

    u32 wBits = (u32)(uintptr_t)first;
    for (u32 i = 0; i < at<u32>(c, 0xf4); ++i) {
        u8 *train = first + i * kTrain;
        at<float>(train, 0x7c) = elapsed;
        SetNodePoints(c, train);
        sorted[i] = train;
        wBits = (u32)(uintptr_t)(train + kTrain);
    }

    while (left) StepTrain(c, type, sorted, left, cap, length, topA, bottom, wBits);

    first = at<u8 *>(c, 0x128);
    for (u32 i = 0; i < at<u32>(c, 0xf4); ++i) {
        u8 *train = first + i * kTrain;
        u32 onChain = 0;
        u8 *car = at<u8 *>(train, 0x28);
        for (u32 k = 0; k < at<u32>(train, 0xc); ++k, car += kCar) {
            if (at<u8>(car, 0) & 1) {
                PlaceCar(c, train, car, k, length);
                onChain += (at<u32>(car, 0) >> 1) & 1;
            } else {
                at<u32>(car, 0x40) = 0;
            }
        }
        SetSound(c, train, onChain, wBits);
        if (!(at<float>(train, 0x74) >= at<float>(at<u8 *>(c, 0x124), 0x74))) at<u8 *>(c, 0x124) = train;
        ClearSpacedFlag(c, type, train, i, spacing);
    }
    at<u32>(c, 0x120) = now;
}
