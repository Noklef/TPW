#include "trakcarupdate.h"

#include <cstdint>

#include "../core/soundset.h"
#include "../math/vec2.h"
#include "../math/x87.h"
#include "../motion/animator.h"
#include "collobj.h"
#include "random.h"
#include "trackrides.h"
#include "trakcarroute.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const TrakCarUpdateEnv kPortEnv = {&Collider_SteerTo, &TrakCar_ChooseRoute, &CollArea_FindNearest, &TrakRide_Unload, &TrakCar_Remove, &Sound_Stop, &Layer_ClearQueue, &Animator_Play};
const TrakCarUpdateEnv *g_env = &kPortEnv;

s32 wrap_sub(s32 a, s32 b) { return (s32)((u32)a - (u32)b); }
s32 magnitude(s32 v) { return v < 0 ? (s32)(0u - (u32)v) : v; }

// Their distance (magnitudes, halved until they fit; fild; fsqrt; __ftol, low half; shifted back).
s32 distance(s32 a, s32 b) {
    u32 halved = 0;
    while (a > 0x8000 || b > 0x7fff) {
        halved++;
        a >>= 1;
        b >>= 1;
    }
    const s32 square = (s32)((u32)a * (u32)a + (u32)b * (u32)b);
    return (s32)((u32)(long long)X87_Sqrt(X87Reg((double)square)).v << halved);
}

bool kind_runs_on_points(s32 kind) { return kind == -14 || kind == -11 || kind == -6 || kind == -3 || kind == -1; }

u32 sent_off(u32 flags) { return flags & 0xffffffa1 | 0x10020; }

// The point in an area the car can reach: a place in the other's area or the module's areas stands, else the nearest area's rim.
void onto_areas(const u8 *other, s32 &x, s32 &y) {
    if (IntArea_Contains(x, y, at<const s32 *>(other, 0x98))) return;
    const u8 *area = global<const u8 *>(0x00877b70);
    if (area) {
        for (s32 left = global<s32>(0x00877b74); left != 0; area += 0x20) {
            if (!(at<u32>(area, 0) & 1)) continue;
            left--;
            if (!(at<u32>(area, 0) & 0x40) && IntArea_Contains(x, y, reinterpret_cast<const s32 *>(area))) return;
        }
    }
    const u8 *const near_area = g_env->findNearest(at<const s32 *>(other, 0x98), x, y, 1);
    const s32 cx = at<s32>(near_area, 4), cy = at<s32>(near_area, 8);
    const s32 dx = wrap_sub(x, cx) / 8, dy = wrap_sub(y, cy) / 8;
    const s32 over = (s32)((u32)distance(magnitude(dx), magnitude(dy)) + 0x20);
    x = (s32)((u32)((s32)((u32)dx * at<u32>(near_area, 0xc)) / over) + (u32)cx);
    y = (s32)((u32)((s32)((u32)dy * at<u32>(near_area, 0xc)) / over) + (u32)cy);
}

void chase(u8 *car, u8 *ride) {
    u32 &flags = at<u32>(car, 0);
    const u32 before = flags;
    const s32 left = at<s32>(car, 0x8c) - 1;
    at<s32>(car, 0x8c) = left;
    const u8 *const other = global<u8 *>(0x00877b68) + at<u32>(car, 0x80) * 0xac;
    if (left < 1 || (at<u32>(other, 0) & 0x40021) != 1) {
        g_env->chooseRoute(ride, car);
        return;
    }
    flags = before | 0x10000;
    s32 speed = distance(magnitude(at<s32>(car, 0x3c)), magnitude(at<s32>(car, 0x40)));
    if (speed < 1) speed = 1;
    const s32 ox = at<s32>(other, 0x34), oy = at<s32>(other, 0x38);
    s32 steps = distance(magnitude(wrap_sub(at<s32>(car, 0x34), ox)), magnitude(wrap_sub(at<s32>(car, 0x38), oy))) / speed;
    if (steps > 0x18) steps = 0x18;
    s32 y = (s32)(at<u32>(other, 0x40) * (u32)steps + (u32)oy);
    s32 x = (s32)(at<u32>(other, 0x3c) * (u32)steps + (u32)ox);
    onto_areas(other, x, y);
    const s32 gap = g_env->steerTo(car, x, y);
    if ((flags & 0x20000) && gap <= at<s32>(at<u8 *>(car, 0x9c), 0x68)) flags &= 0xfffdffff;
}

void leave(u8 *car, u8 *ride) {
    u32 &flags = at<u32>(car, 0);
    if (kind_runs_on_points(at<s32>(ride, 0))) {
        if (g_env->unload(car, 1) == 0) {
            at<u32>(at<u8 *>(car, 0x9c), 0x50) = 1;
            const u32 motion = at<u32>(car, 0x10);
            flags &= 0xffffff81;
            if (motion != 0xc) {
                at<u32>(car, 0x10) = 0xc;
                u8 *const *const instances = reinterpret_cast<u8 *const *>(0x007a4610);
                if (at<u32>(car, 8)) g_env->clearQueue(instances[at<u32>(car, 8)], 0);
                if (at<u32>(car, 8)) g_env->play(instances[at<u32>(car, 8)], 0xc, 0, 2, 1.0f, 0);
            }
        }
        if (at<u32>(car, 0x20)) g_env->stopSound(at<u32>(car, 0x20));
        return;
    }
    const s32 gap = g_env->steerTo(car, at<s32>(ride, 0x90), at<s32>(ride, 0x94));
    if ((flags & 0x20000) && gap <= at<s32>(at<u8 *>(car, 0x9c), 0x68)) {
        flags = flags & 0xfffdffff | 0x10000;
        return;
    }
    if (gap < at<s32>(at<u8 *>(car, 0x9c), 0x68)) g_env->remove(car);
}

// The route followed from a node: true when it has finished the frame (0x10000 set or nothing more to do).
void next_node(u8 *car, const u8 *node) {
    u8 *const ride = at<u8 *>(car, 0x9c);
    do {
        node = at<const u8 *>(node, 0x14);
        if (!node) node = at<const u8 *>(ride, 0xa8);
    } while (at<u8>(node, 0) & 2);
    at<u32>(car, 0x7c) = at<u32>(node, 4);
    at<u32>(car, 0x78) = at<u32>(car, 0x74);
    const u32 span = at<u32>(ride, 0x68);
    at<s32>(car, 0x74) = (s32)((u32)((s32)span / 2) - (u32)World_Rand(global<u8 *>(0x007cf83c)) % span);
    at<u32>(car, 0) |= 0x10000;
}

void follow_nodes(u8 *car) {
    u32 &flags = at<u32>(car, 0);
    const u8 *const node = global<u8 *>(0x00877b78) + at<u32>(car, 0x7c) * 0x1c;
    if (!(at<u8>(node, 0) & 1)) return;
    const s32 gap = g_env->steerTo(car, at<s32>(node, 8), at<s32>(node, 0xc));
    const u32 now = flags;
    if ((now & 0x20000) && gap <= at<s32>(at<u8 *>(car, 0x9c), 0x68)) {
        flags = now & 0xfffdffff | 0x10000;
        return;
    }
    if ((now & 0x48000) == 0x48000) {
        g_env->chooseRoute(at<u8 *>(car, 0x9c), car);
        flags |= 0x10000;
        return;
    }
    u8 *const ride = at<u8 *>(car, 0x9c);
    const s32 reach = at<s32>(ride, 0x68);
    if (gap > reach) return;
    if (--at<s32>(car, 0x8c) == 0) {
        g_env->chooseRoute(ride, car);
        flags |= 0x10000;
        return;
    }
    if (!(now & 8)) {
        if (now & 0x10) at<u32>(car, 0x7c) = (u32)World_Rand(global<u8 *>(0x007cf83c)) % global<u32>(0x00877b7c);
    } else if (now & 0x40) {
        if (gap <= reach / 4) {
            at<u32>(car, 0x7c) = at<u32>(at<u8 *>(ride, 0xa4), 4);
            flags = now | 0x90000;
            return;
        }
    } else {
        if (at<u8>(node, 0) & 4) {
            const u32 state = at<u32>(ride, 0x50);
            if (state == 3 || state == 0 || ++at<s32>(car, 0x88) == 0) {
                const s32 kind = at<s32>(ride, 0);
                if ((kind == -10 || kind == -9 || kind == -7 || kind == -4) && state == 2) at<u32>(ride, 0x50) = 3;
                at<u32>(car, 0x74) = 0;
                at<u32>(car, 0x78) = 0;
                flags = sent_off(flags);
            } else {
                g_env->chooseRoute(ride, car);
            }
        }
        if (const s32 *const area = at<const s32 *>(node, 0x18)) {
            if (!IntArea_Contains(at<s32>(car, 0x34), at<s32>(car, 0x38), area)) {
                at<u32>(car, 0x78) = 0;
                at<u32>(car, 0x74) = 0;
                flags |= 0x10000;
                return;
            }
            at<const s32 *>(car, 0x98) = area;
        }
        next_node(car, node);
        return;
    }
    flags |= 0x10000;
}

}  // namespace

void TrakCarUpdate_SetEnv(const TrakCarUpdateEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

void __cdecl TrakCar_Update(u8 *car) {
    u32 &flags = at<u32>(car, 0);
    u8 *const ride = at<u8 *>(car, 0x9c);
    at<s32>(car, 0x90)++;
    if (at<s32>(car, 0x88) >= 0 && at<u32>(ride, 0x50) == 2 && at<u32>(ride, 0x54) != 2 && --at<s32>(car, 0x88) == 0) {
        if (kind_runs_on_points(at<s32>(ride, 0))) flags &= 0xffffbfff;
        flags = sent_off(flags);
    }
    if (flags & 4) {
        chase(car, ride);
    } else if (flags & 0x20) {
        leave(car, ride);
    } else if ((flags & 0x58) && global<u32>(0x00877b78)) {
        follow_nodes(car);
    }
}
}
