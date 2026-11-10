#include "trakridecars.h"

#include <cstdint>

#include "../math/vec2.h"
#include "../math/x87.h"
#include "../motion/animator.h"
#include "../render/particles.h"
#include "objectdefs.h"
#include "rides.h"
#include "trackrides.h"
#include "trakcarroute.h"
#include "collobj.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TrakRideCarsEnv kPortEnv = {&TrakCar_ChooseRoute, &Particle_Spawn, &Layer_ClearQueue, &Animator_Play, &TrakRide_Load, &Ride_Create, &TrakCar_Remove};
const TrakRideCarsEnv *g_env = &kPortEnv;

const u32 kCarSize = 0xac;

// The ride the handle indexes (none when the module is not loaded).
u8 *ride_of(u32 handle) {
    return gTrackPools.tag == 0x4a454647 ? reinterpret_cast<u8 *>((uintptr_t)gTrackPools.rides + (handle & 0xff) * 0xd0) : nullptr;
}
// Whether the handle names it (not loaded: only a handle of 0).
bool names(u32 handle, const u8 *ride) { return handle == (gTrackPools.tag == 0x4a454647 ? (handle & 0xff) | at<u32>(ride, 0) << 8 : 0); }

u8 *instance_of(u32 id) { return reinterpret_cast<u8 *const *>(0x007a4610)[id]; }

void set_motion(u8 *car, u32 motion, u32 mode) {
    at<u32>(car, 0x10) = motion;
    if (at<u32>(car, 8)) g_env->clearQueue(instance_of(at<u32>(car, 8)), 0);
    if (at<u32>(car, 8)) g_env->play(instance_of(at<u32>(car, 8)), motion, 0, mode, 1.0f, 0);
}

s32 by1024(float v) { return (s32)(long long)(X87Reg(v) * 1024.0f).v; }

// The first used car of the module's areas holding the place, as TrakCar's area pass; none: 0.
const u8 *area_at(s32 x, s32 y) {
    const u8 *area = gTrackPools.nodes;
    if (!area) return nullptr;
    for (s32 left = gTrackPools.nodeCount; left != 0; area += 0x20) {
        if (!(at<u32>(area, 0) & 1)) continue;
        left--;
        if (!(at<u32>(area, 0) & 0x40) && IntArea_Contains(x, y, reinterpret_cast<const s32 *>(area))) return area;
    }
    return nullptr;
}

}  // namespace

void TrakRideCars_SetEnv(const TrakRideCarsEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

void __cdecl TrakRide_Break(u32 handle) {
    u8 *const ride = ride_of(handle);
    if (!names(handle, ride)) return;
    at<u32>(ride, 0x54) = 2;
    const s32 kind = at<s32>(ride, 0);
    if (!(kind > -15 && (kind < -2 || kind == -1))) return;
    u8 *car = gTrackPools.cars;
    for (s32 left = gTrackPools.carCount; left != 0; car += kCarSize) {
        if (!(at<u32>(car, 0) & 1)) continue;
        left--;
        if (at<u8 *>(car, 0x9c) != ride || !(at<u32>(car, 0) & 0x4000) || at<s32>(car, 0x24) == -1 || at<s32>(car, 0x2c) != -1) continue;
        const u8 *const states = at<u8 *>(at<u8 *>(instance_of(at<u32>(car, 8)), 8), 0x28);
        const u8 *const entry = at<u8 *>(at<u8 *>(states, 4), at<u32>(car, 0x24) * 0x14 + 4);
        float px = 0, py = 0, pz = 0;
        if (entry) {
            px = at<float>(entry, 0x30);
            py = at<float>(entry, 0x34);
            pz = at<float>(entry, 0x38);
        }
        const s32 z = by1024(pz), y = by1024(py), x = by1024(px);
        at<s32>(car, 0x2c) = g_env->spawn(2, x, y, z);
        if (at<s32>(at<u8 *>(car, 0x9c), 0) == -1) set_motion(car, 0xc, 2);
    }
}

void __cdecl TrakRide_StartBump(u32 handle) {
    u8 *const ride = ride_of(handle);
    if (!names(handle, ride)) return;
    const s32 kind = at<s32>(ride, 0);
    if (!(kind == -14 || kind == -11 || kind == -10 || kind == -9 || kind == -7 || kind == -6 || kind == -4 || kind == -3 || kind == -1)) return;
    if (at<u32>(ride, 0x50) != 1) return;
    at<u32>(ride, 0x50) = 2;
    u8 *car = gTrackPools.cars;
    for (s32 left = gTrackPools.carCount; left != 0; car += kCarSize) {
        if (!(at<u32>(car, 0) & 1)) continue;
        left--;
        if (at<u8 *>(car, 0x9c) != ride) continue;
        at<u32>(car, 0) &= 0xfff7ffbf;
        at<u32>(car, 0x88) = at<u32>(ride, 4);
        g_env->chooseRoute(ride, car);
        if (at<s32>(ride, 0) == -1 && (at<u32>(car, 0) & 0x4000)) set_motion(car, 5, 1);
    }
}

u32 __cdecl TrakRide_LoadCar(u32 handle) {
    u8 *const ride = ride_of(handle);
    if (!names(handle, ride) || at<u32>(ride, 0x50) == 0) return 0;
    s32 left = gTrackPools.carCount;
    if (left == 0) return 0;
    u8 *car = gTrackPools.cars;
    for (;; car += kCarSize) {
        if (at<u32>(car, 0) & 1) {
            left--;
            if (at<u8 *>(car, 0x9c) == ride && at<u32>(car, 0x30) == 0) break;
        }
        if (left == 0) return 0;
    }
    at<u32>(car, 0x30) = at<u32>(ride, 0xc4);
    at<u32>(ride, 0xc4) = 0;
    at<u32>(car, 0) |= 0x84000;
    at<u32>(car, 0x88) = at<u32>(ride, 4);
    g_env->load(car, 1);
    g_env->chooseRoute(ride, car);
    return 1;
}

u8 *__cdecl TrakRide_AddCar(u32 handle, u32 flags) {
    u8 *const ride = ride_of(handle);
    if (!names(handle, ride) || at<s32>(ride, 0x5c) >= at<s32>(ride, 0x64) || at<s32>(ride, 0x5c) >= 0x40 || at<u32>(ride, 0x50) == 0) return nullptr;
    u8 *const car = TrakPool_FirstFree();
    if (!car) return nullptr;
    const u32 y = at<u32>(ride, 0x78), x = at<u32>(ride, 0x74), owner = at<u32>(ride, 8);
    gTrackPools.carCount++;
    for (u32 off = 0; off < kCarSize; off += 4) at<u32>(car, off) = 0;
    at<u32>(car, 0x38) = y;
    at<u32>(car, 0x64) = owner;
    at<u32>(car, 0) = 1;
    at<u32>(car, 0x34) = x;
    at<s32>(car, 0x80) = -1;
    at<s32>(car, 0x7c) = -1;
    at<s32>(car, 0x2c) = -1;
    at<s32>(car, 0x28) = -1;
    at<s32>(car, 0x24) = -1;
    at<u8 *>(car, 0x9c) = ride;
    at<u32>(car, 0) |= 0x4000;
    at<s32>(ride, 0x5c)++;
    at<u32>(car, 0x88) = at<u32>(ride, 4);
    at<const u8 *>(car, 0x98) = area_at(at<s32>(ride, 0x74), at<s32>(ride, 0x78));
    at<u32>(car, 0x30) = at<u32>(ride, 0xc4);
    at<u32>(ride, 0xc4) = 0;
    if (flags != 0) {
        at<u32>(car, 0x80) = 0;
        at<u32>(car, 0x7c) = 0;
        at<u32>(car, 0) |= flags;
        return car;
    }
    at<u32>(car, 0) |= 0x100000;
    at<s32>(car, 4) = (s32)((u32)(at<s32>(ride, 0x5c) % at<s32>(ride, 0x18)) + at<u32>(ride, 0x14));
    const u8 *const definition = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(ride, 0x10));
    const u32 type = at<u32>(definition, 0x4bc + at<u32>(car, 4) * 4);
    const u32 made = g_env->createRide(type, 0x101, nullptr, 0, 0, 0, 0, 0, -1, 0);
    at<u32>(car, 8) = made;
    if (made) g_env->play(instance_of(made), 0xc, 0, 2, 1.0f, 0);
    at<u32>(car, 0x10) = 0xc;
    if (at<u32>(ride, 0x58) == 0) {
        at<u8 *>(ride, 0x58) = car;
        at<u32>(car, 0) |= 0x400000;
    }
    g_env->load(car, 0);
    g_env->chooseRoute(ride, car);
    at<u32>(car, 0) |= 0x4000000;
    return car;
}

u32 __cdecl TrakRide_RemoveCar(u32 handle, u32 empty) {
    const u8 *const ride = ride_of(handle);
    if (names(handle, ride)) {
        u8 *car = gTrackPools.cars;
        for (s32 left = gTrackPools.carCount; left != 0; car += kCarSize) {
            if (!(at<u32>(car, 0) & 1)) continue;
            left--;
            if (at<const u8 *>(car, 0x9c) == ride && (empty == 0 || at<u32>(car, 0x30) == 0)) {
                g_env->remove(car);
                return 1;
            }
        }
    }
    if (empty != 0) TrakRide_RemoveCar(handle, 0);
    return 0;
}
}
