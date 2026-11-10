#include "trakcarroute.h"

#include <cstdint>

#include "../core/soundplay.h"
#include "../core/soundset.h"
#include "../math/vec2.h"
#include "../math/x87.h"
#include "../model/parts.h"
#include "objectdefs.h"
#include "random.h"
#include "rides.h"
#include "things.h"
#include "trackrides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TrakCarRouteEnv kPortEnv = {&Model_FindPartByKey4, &Ride_Create, &Sound_PlayRideElement, &Sound_SetPosition, &IntArea_Overlaps};
const TrakCarRouteEnv *g_env = &kPortEnv;

const u32 kNew = 0x100000;
const float kToSound = 0.00330360071f;   // 0x700f20

u32 Rand() { return (u32)World_Rand(global<u8 *>(0x007cf83c)); }
u8 *Pool() { return global<u8 *>(0x00877b68); }

void finish(u8 *car) { at<u32>(car, 0) &= ~kNew; }

// x / 256, towards zero.
s32 per256(s32 x) { return (x + (s32)((u32)(x >> 31) & 0xff)) >> 8; }
s32 abs32(s32 x) { return x < 0 ? (s32)(0u - (u32)x) : x; }

// Whether the place +0x34 / +0x38 of the car is free of the other used cars (their sizes +0x64 added).
bool clash(const u8 *car) {
    u8 *other = Pool();
    for (u32 left = global<u32>(0x00877b6c); left != 0; other += 0xac) {
        if (!(at<u32>(other, 0) & 1)) continue;
        --left;
        if (other == car) continue;
        s32 dx = abs32((s32)((u32)at<s32>(other, 0x34) - (u32)at<s32>(car, 0x34)));
        s32 dy = abs32((s32)((u32)at<s32>(other, 0x38) - (u32)at<s32>(car, 0x38)));
        u32 halved = 0;
        while (dx > 0x8000 || dy > 0x7fff) {
            ++halved;
            dx >>= 1;
            dy >>= 1;
        }
        const s32 squares = (s32)((u32)dx * (u32)dx + (u32)dy * (u32)dy);
        const u32 distance = (u32)(s32)(long long)X87_Sqrt(X87Reg((double)squares)).v << (halved & 31);
        if ((s32)((u32)at<s32>(other, 0x64) + (u32)at<s32>(car, 0x64)) > (s32)distance) return true;
    }
    return false;
}

// Up to 100 tries at a free place in the ride's area.
void place(const u8 *ride, u8 *car) {
    const s32 *const sines = reinterpret_cast<const s32 *>(0x00877358);
    for (s32 tries = 100;;) {
        const u32 radius = Rand() % at<u32>(at<const u8 *>(ride, 0xc0), 0xc);
        const u32 angle = Rand() & 0x1ff;
        const s32 s = sines[angle], c = sines[(angle + 0x80) & 0x1ff];
        at<s32>(car, 0x34) = (s32)((u32)per256((s32)((u32)c * radius)) + (u32)at<s32>(ride, 0x74));
        at<s32>(car, 0x38) = (s32)((u32)at<s32>(ride, 0x78) - (u32)per256((s32)(radius * (u32)s)));
        if (--tries == 0 || !clash(car)) return;
    }
}

// __ftol (fild n; fmul 0x700f20).
s32 soundPlace(s32 n) { return (s32)(long long)X87_MulIntDouble(n, (double)kToSound); }

// The car's sound: its ride's object's id (TrakRide_FindThing's search), at the car's place.
void startSound(u8 *car) {
    const u8 *const ride = at<const u8 *>(car, 0x9c);
    const u32 name =
        global<u32>(0x00877b58) == 0x4a454647 ? (u32)((s32)((uintptr_t)ride - global<uintptr_t>(0x00877b60)) / 0xd0) | at<u32>(ride, 0) << 8 : 0;
    u16 handle;
    const u16 found = *TrakRide_FindThing(&handle, name);
    const s32 id = found ? at<s32>(Thing_ByHandle(found), 0x24) : 0;
    const u32 sound = g_env->playSound(id, 0, gSoundCategories.themeRides, 0, 0, 0, 0);
    at<u32>(car, 0x20) = sound;
    if (sound) {
        const s32 x = soundPlace(at<s32>(car, 0x34));
        const s32 z = soundPlace(at<s32>(car, 0x38));
        at<u32>(car, 0x20) = g_env->setSoundPosition(sound, x, 0, z);
    }
}

// A ride object of the ride's definition, for the car (+0xc).
void createRide(const u8 *ride, u8 *car) {
    const u8 *const def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(ride, 0x10));
    at<u32>(car, 0xc) = g_env->createRide(at<u32>(def, 0x4bc), 0x101, nullptr, 0, 0, 0, 0, 0, -1, 0);
}

// The next of the node, past the end the route's first.
const u8 *nextNode(const u8 *ride, const u8 *node) {
    const u8 *const next = at<const u8 *>(node, 0x14);
    return next ? next : at<const u8 *>(ride, 0xa8);
}

const u8 *arrayNode(const u8 *car) { return global<const u8 *>(0x00877b78) + (u32)at<s32>(car, 0x7c) * 0x1c; }

// A +0x7c outside the array becomes the route's start's point.
void holdPoint(const u8 *ride, u8 *car) {
    const s32 point = at<s32>(car, 0x7c);
    if (point < 0 || point >= global<s32>(0x00877b7c)) at<u32>(car, 0x7c) = at<u32>(at<const u8 *>(ride, 0xa4), 4);
}

// A random other used car on the ride whose area overlaps, not flagged 0x20: its index; whether there was one.
bool partner(u8 *car, u32 &index) {
    u8 *list = nullptr, *other = Pool();
    u32 count = 0;
    for (u32 left = global<u32>(0x00877b6c); left != 0; other += 0xac) {
        const u32 flags = at<u32>(other, 0);
        if (!(flags & 1)) continue;
        if (other != car && !(flags & 0x20) && at<u32>(car, 0x9c) == at<u32>(other, 0x9c) &&
            g_env->areasOverlap(at<const s32 *>(car, 0x98), at<const s32 *>(other, 0x98))) {
            at<u8 *>(other, 0x94) = list;
            ++count;
            list = other;
        }
        --left;
    }
    if (count == 0) return false;
    for (u32 k = Rand() % count; k != 0; --k) list = at<u8 *>(list, 0x94);
    index = (u32)((s32)((uintptr_t)list - (uintptr_t)Pool()) / 0xac);
    return list != nullptr;
}

// The ride kinds that wander: placed when new, a sound, a partner or the next point.
void wander(const u8 *ride, u8 *car) {
    if (at<u32>(car, 0) & kNew) {
        place(ride, car);
        at<u32>(car, 0) &= ~0x4000u;
        const u32 phase = Rand() & 0x1ff;
        at<u32>(car, 0x54) = phase;
        at<u32>(car, 0x50) = phase;
        const u8 *const *const instances = reinterpret_cast<const u8 *const *>(0x007a4610);
        at<u32>(car, 0x24) = g_env->findPart(instances[at<u32>(car, 8)], 0x100, 2);
        at<u32>(car, 0x28) = g_env->findPart(instances[at<u32>(car, 8)], 0x100, 1);
        if (at<s32>(ride, 0) == -1) {
            at<u32>(car, 0) |= 0x3000000;
            createRide(ride, car);
        }
        at<s32>(car, 0xa4) = -1;
    }
    if (!at<u32>(car, 0x20) && (at<u32>(car, 0) & 0x404000) == 0x404000) {
        if (at<s32>(at<const u8 *>(car, 0x9c), 0x50) == 2 && !(at<u8>(car, 0) & 0x20)) startSound(car);
    }
    if (!(at<u32>(car, 0) & 0x40004) && (Rand() & 0xf) < 3) {
        u32 index;
        if (at<s32>(ride, 0x5c) > 1 && partner(car, index)) {
            at<u32>(car, 0) = (at<u32>(car, 0) & 0xfffbff85) | 4;
            at<u32>(car, 0x80) = index;
            at<u32>(car, 0x8c) = 0x5a;
            at<u32>(car, 0) |= 0x8000;
            finish(car);
            return;
        }
    }
    const u8 *node;
    u32 steps;
    if ((at<u8>(car, 0) & 8) && (Rand() & 0xf) < 4) {
        node = arrayNode(car);
        steps = 1;
    } else {
        steps = Rand() % at<u32>(ride, 0xac);
        node = at<const u8 *>(ride, 0xa8);
    }
    at<u32>(car, 0) = (at<u32>(car, 0) & 0xfffbff89) | 0x10008;
    for (; steps != 0; --steps) node = nextNode(ride, node);
    at<u32>(car, 0x7c) = at<u32>(node, 4);
    at<u32>(car, 0) |= 0x8000;
    at<u32>(car, 0x8c) = 3;
    finish(car);
}

// The ride kinds that follow the route by the ride's state.
void follow(const u8 *ride, u8 *car) {
    at<u32>(car, 0) &= 0xfffbff81;
    at<u32>(car, 0x8c) = 0;
    if (at<u32>(car, 0) & kNew) {
        startSound(car);
        const u8 *const *const instances = reinterpret_cast<const u8 *const *>(0x007a4610);
        at<u32>(car, 0x24) = g_env->findPart(instances[at<u32>(car, 8)], 0x100, 1);
        if (at<u32>(car, 0) & 0x2000) createRide(ride, car);
    }
    const s32 state = at<s32>(ride, 0x50);
    if (state == 1) {
        const u8 *node = at<const u8 *>(ride, 0xa8);
        for (u32 left = at<u32>(ride, 0x5c); left != 0;) {
            while (!(*(const volatile u8 *)node & 2)) {
            }   // (sic: forever)
            if (--left == 0) break;
            node = at<const u8 *>(node, 0x14);
        }
        at<u32>(car, 0x7c) = at<u32>(node, 4);
        at<u32>(car, 0) |= 0x10048;
    } else if (state == 2) {
        at<u32>(car, 0) |= 0x10008;
        holdPoint(ride, car);
        at<u32>(car, 0x8c) = 0;
        const u32 spread = 3u - (Rand() & 7);
        at<u32>(car, 0x68) = spread * (u32)(at<s32>(ride, 0x20) / 9);
    }
    finish(car);
}

// The ride kinds that start the route when new.
void start(const u8 *ride, u8 *car) {
    at<u32>(car, 0x8c) = 0;
    at<u32>(car, 0) &= 0xfffbff81;
    if (at<u32>(car, 0) & kNew) {
        startSound(car);
        at<u32>(car, 0x7c) = at<u32>(at<const u8 *>(ride, 0xa4), 4);
    }
    at<u32>(car, 0) |= 0x10008;
    holdPoint(ride, car);
    at<u32>(car, 0x8c) = 0;
    at<u32>(car, 0x68) = 0;
    finish(car);
}

}  // namespace

void TrakCarRoute_SetEnv(const TrakCarRouteEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

void __cdecl TrakCar_ChooseRoute(u8 *ride, u8 *car) {
    switch (at<s32>(ride, 0)) {
    case -14:
    case -11:
    case -6:
    case -3:
    case -1:
        wander(ride, car);
        return;
    case -10:
    case -9:
        if (at<u32>(car, 0) & kNew) at<u32>(car, 0) |= 0x3c00;
        follow(ride, car);
        return;
    case -7:
    case -4:
        follow(ride, car);
        return;
    case -13:
    case -12:
    case -8:
    case -5:
        start(ride, car);
        return;
    case -2: {
        const u8 *const from = (at<u32>(car, 0) & 8) ? arrayNode(car) : at<const u8 *>(ride, 0xa8);
        at<u32>(car, 0) = (at<u32>(car, 0) & 0xfffbff89) | 0x10008;
        at<u32>(car, 0x7c) = at<u32>(nextNode(ride, from), 4);
        at<u32>(car, 0x8c) = 0;
        finish(car);
        return;
    }
    default:
        at<u32>(car, 0) = (at<u32>(car, 0) & 0xffffff91) | 0x10010;
        at<u32>(car, 0x7c) = Rand() % global<u32>(0x00877b7c);
        finish(car);
        return;
    }
}
}
