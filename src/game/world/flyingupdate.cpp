#include "flyingupdate.h"

#include <cstdint>

#include "../core/soundset.h"
#include "../math/x87.h"
#include "../terrain/land.h"
#include "flyinglaunch.h"
#include "flyingrides.h"
#include "random.h"
#include "ridelist.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const FlyingUpdateEnv kPortEnv = {&FlyingRide_Launch, &FlyingRide_PlaySlotMotion, &Rides_GetLinkedElement, &Sound_SetParam, &Sound_SetPosition,
                                  &Terrain_SurfaceHeight, &FlyingRide_NextCameraCorner, &FlyingRide_PickWaypoint, &FlyingRide_ClearSlot};
const FlyingUpdateEnv *g_env = &kPortEnv;

const u32 kSlots = 20, kSlotSize = 0xe4;

s32 add(s32 a, s32 b) { return (s32)((u32)a + (u32)b); }
s32 sub(s32 a, s32 b) { return (s32)((u32)a - (u32)b); }
s32 mul(s32 a, s32 b) { return (s32)((u32)a * (u32)b); }
s32 bound(u32 address) { return global<s32>(address); }

// The magnitude of a World_Rand (wrapping).
s32 Rand() {
    const s32 r = World_Rand(global<u8 *>(0x007cf83c));
    return r < 0 ? (s32)(0u - (u32)r) : r;
}

// The sine table's entry at an angle of 4096 to the turn (wrapped), times by, / 1024.
s32 sine(s32 angle, s32 by) { return mul(gFlyingrides_8791c8[(u32)angle & 0xfff], by) / 1024; }
s32 cosine(s32 angle, s32 by) { return sine(add(angle, 0x400), by); }

// __ftol's low half of the root of a sum of squares (a wrapped sum's NaN: 0).
s32 root(s32 square) { return (s32)(u32)(long long)X87_Sqrt(X87Reg((double)square)).v; }
s32 square(s32 d) { return mul(d, d); }

// The distance across the ground to the target (+0x4c, +0x54), __ftol'd in 32nds.
s32 target_root(const u8 *slot) {
    return root(add(square(sub(at<s32>(slot, 4), at<s32>(slot, 0x4c)) / 32), square(sub(at<s32>(slot, 0xc), at<s32>(slot, 0x54)) / 32)));
}
bool near_target(const u8 *slot) { return mul(target_root(slot), 32) < 5000; }

// __ftol of atan2(a, b) (the integers loaded by fild) times by.
s32 bearing(s32 a, s32 b, double by) { return (s32)(u32)(long long)X87_MulDouble(X87_Atan2((double)a, (double)b), by); }

// The surface's height (by __ftol, times 300) at a place's cell, clamped by hand; 0 off the playfield.
s32 surface_at(s32 x, s32 z) {
    if (x >= bound(0x007660b8) || z >= bound(0x007660c0) || x < bound(0x007660bc) || z < bound(0x007660c4)) return 0;
    s32 cx = x / 3000, cz = z / 3000;
    if (cx < 0) cx = 0;
    if (cz < 0) cz = 0;
    if (cx >= bound(0x007a0314)) cx = bound(0x007a0314) - 1;
    if (cz >= bound(0x007a0374)) cz = bound(0x007a0374) - 1;
    return mul((s32)(u32)(long long)g_env->surfaceHeight(cx, cz, nullptr), 300);
}

// 5000 over home or the ground, the higher (the ground read again when it is the ground).
s32 climb_height(const u8 *record, const u8 *slot) {
    const s32 ground = add(surface_at(at<s32>(slot, 0x4c), at<s32>(slot, 0x54)), 5000);
    const s32 home = add(at<s32>(record, 0x18), 5000);
    return home <= ground ? add(surface_at(at<s32>(slot, 0x4c), at<s32>(slot, 0x54)), 5000) : home;
}

// The sound's parameter: the element looked up whether there is a sound or not.
void sound_param(const u8 *record, u8 *slot, u32 value) {
    const u32 linked = g_env->linkedElement(at<s32>(record, 0x54), 10);
    if (const u32 sound = at<u32>(slot, 0xe0)) at<u32>(slot, 0xe0) = g_env->setParam(sound, linked, value);
}

// The flying motion and sound: bank 5, 0x1e; with +0xb8, bank 9, 0x3c.
void fly_on(s32 index, u32 k, const u8 *record, u8 *slot) {
    const bool b8 = at<u32>(slot, 0xb8) != 0;
    g_env->playMotion(index, k, b8 ? 9 : 5, 0, 1);
    sound_param(record, slot, b8 ? 0x3c : 0x1e);
}

// A coordinate's square of 20000: the first below it left of 0 (one too far on a multiple, sic).
s32 square_of(s32 v) { return v < 0 ? sub(mul(v / 20000, 20000), 20000) : mul(v / 20000, 20000); }

void off_playfield(const u8 *record, u8 *slot) {
    at<s32>(slot, 0x50) = 14000;
    const s32 hx = bound(0x007660b8), lx = bound(0x007660bc), hz = bound(0x007660c0), lz = bound(0x007660c4);
    if ((Rand() & 1) == 0) {
        const s32 r = Rand();
        at<s32>(slot, 0x54) = add(mul(r, 100) % sub(hz, lz), lz);
        at<s32>(slot, 0x4c) = (Rand() & 1) == 0 ? sub(lx, 45000) : add(hx, 45000);
    } else {
        const s32 r = Rand();
        at<s32>(slot, 0x4c) = add(mul(r, 100) % sub(hx, lx), lx);
        at<s32>(slot, 0x54) = (Rand() & 1) == 0 ? sub(lz, 45000) : add(hz, 45000);
    }
    at<s32>(slot, 0x7c) = mul(at<s32>(record, 0x28), 2);
}

void by_state(s32 index, u32 k, u8 *record, u8 *slot) {
    s32 &state = at<s32>(slot, 0x80);
    switch (state) {
    case 1:
        at<s32>(slot, 0x14) /= 2;
        sound_param(record, slot, 0x50);
        if (at<u32>(record, 0x3c) == 0 && at<u32>(slot, 0xb8) == 0 && (at<u32>(record, 0x38) != 0 || k != 0 || at<s32>(record, 0xc) != 1)) {
            state = 2;
            at<s32>(slot, 0x7c) = add(Rand() % 100, at<s32>(record, 0x28));
            at<s32>(slot, 0x84) = at<s32>(record, 0x10);
            const s32 angle = at<u32>(record, 0x44) == 0 ? at<s32>(slot, 0x10) : at<s32>(record, 0x40);
            at<s32>(slot, 0x4c) = add(cosine(angle, 20000), at<s32>(record, 0x14));
            at<s32>(slot, 0x54) = add(sine(angle, 20000), at<s32>(record, 0x1c));
            at<s32>(slot, 0x50) = climb_height(record, slot) < 14000 ? 14000 : climb_height(record, slot);
        }
        break;
    case 2:
        g_env->playMotion(index, k, 4, 0, 1);
        sound_param(record, slot, 0xf);
        if (at<s32>(slot, 0x50) <= at<s32>(slot, 8)) {
            at<u32>(record, 0x2c) = 0;
            if (at<u32>(slot, 0xb4) == 0) {
                state = 3;
                at<s32>(slot, 0x58) = sub(square_of(at<s32>(slot, 0x4c)), 20000);
                at<s32>(slot, 0x5c) = at<s32>(slot, 0x50);
                at<s32>(slot, 0x60) = square_of(at<s32>(slot, 0x54));
            } else {
                state = 4;
            }
        }
        break;
    case 3:
        fly_on(index, k, record, slot);
        if (at<u32>(slot, 0xb8) != 0) {
            if (at<s32>(record, 0x34) == 3) at<u32>(slot, 0x84) = 0;
            at<s32>(slot, 0x7c) = at<s32>(record, 0x28) / 2;
        }
        if (at<s32>(slot, 0x84) < 1 || at<u32>(slot, 0xb4) != 0) {
            state = 4;
            g_env->pickWaypoint(slot);
            at<s32>(slot, 0x50) = 14000;
        } else if (near_target(slot)) {
            g_env->pickWaypoint(slot);
        }
        break;
    case 4:
        fly_on(index, k, record, slot);
        if (near_target(slot)) {
            if (at<u32>(slot, 0xb4) == 0 || at<u32>(slot, 0x8c) != 0) {
                state = 8;
                g_env->nextCorner(reinterpret_cast<s32 *>(slot));
            } else {
                state = 9;
                off_playfield(record, slot);
            }
        }
        break;
    case 5:
        fly_on(index, k, record, slot);
        if (near_target(slot)) {
            state = 6;
            const s32 angle = at<s32>(slot, 0xbc), out = at<s32>(record, 0x30);
            at<s32>(slot, 0x4c) = sub(at<s32>(record, 0x14), cosine(angle, out));
            at<s32>(slot, 0x50) = add(at<s32>(record, 0x18), 5000);
            at<s32>(slot, 0x54) = sub(at<s32>(record, 0x1c), sine(angle, out));
        }
        if (at<u32>(slot, 0xb4) != 0 && at<u32>(slot, 0x8c) == 0) {
            state = 4;
            at<s32>(slot, 0x50) = 14000;
            at<u32>(record, 0x2c) = 0;
        }
        break;
    case 6:
        fly_on(index, k, record, slot);
        if (near_target(slot)) {
            state = 7;
            at<s32>(slot, 0x7c) = at<u32>(record, 0x4c) == 0 ? at<s32>(record, 0x28) : at<s32>(record, 0x28) / 2;
            at<s32>(slot, 0x4c) = at<s32>(record, 0x14);
            at<s32>(slot, 0x50) = at<s32>(record, 0x18);
            at<s32>(slot, 0x54) = at<s32>(record, 0x1c);
        }
        break;
    case 7:
        if (at<s32>(record, 0x18) < at<s32>(slot, 8)) {
            g_env->playMotion(index, k, 6, 0, 0);
            sound_param(record, slot, at<u32>(slot, 0xb8) == 0 ? 0x1e : 0x3c);
        } else {
            at<s32>(slot, 8) = at<s32>(record, 0x18);
            state = 1;
            at<u32>(record, 0x3c) = 1;
            at<u32>(slot, 0x18) = 0;
            at<u32>(slot, 0x7c) = 0;
            at<u32>(slot, 0x74) = 0;
            if (at<s32>(slot, 0x8c) > 0) at<s32>(slot, 0x8c) = (s32)(0u - at<u32>(slot, 0x8c));
            g_env->playMotion(index, k, 3, 0, 0);
            sound_param(record, slot, 0x50);
        }
        break;
    case 8:
        fly_on(index, k, record, slot);
        if (at<u32>(slot, 0x8c) == 0 && at<u32>(slot, 0xb4) != 0) {
            state = 4;
            g_env->nextCorner(reinterpret_cast<s32 *>(slot));
            at<s32>(slot, 0x50) = 14000;
        }
        if (near_target(slot)) g_env->nextCorner(reinterpret_cast<s32 *>(slot));
        if (at<u32>(record, 0x2c) == 0) {
            at<u32>(record, 0x2c) = 1;
            const s32 out = at<s32>(record, 0x30);
            const s32 high = add(at<s32>(record, 0x18), 5000);
            if (at<u32>(record, 0x44) == 0) {
                at<s32>(slot, 0xbc) = bearing(sub(at<s32>(record, 0x1c), at<s32>(slot, 0xc)), sub(at<s32>(record, 0x14), at<s32>(slot, 4)), 652.0);
                state = 6;
                const s32 angle = at<s32>(slot, 0xbc);
                at<s32>(slot, 0x4c) = sub(at<s32>(record, 0x14), cosine(angle, out));
                at<s32>(slot, 0x50) = high < 14000 ? 14000 : high;
                at<s32>(slot, 0x54) = sub(at<s32>(record, 0x1c), sine(angle, out));
            } else {
                const s32 angle = at<s32>(record, 0x40);
                at<s32>(slot, 0xbc) = angle;
                state = 5;
                at<s32>(slot, 0x4c) = sub(at<s32>(record, 0x14), mul(gFlyingrides_8791c8[(u32)add(angle, 0x400) & 0xfff], mul(out, 6)) / 1024);
                at<s32>(slot, 0x50) = high < 14000 ? 14000 : high;
                at<s32>(slot, 0x54) = sub(at<s32>(record, 0x1c), mul(gFlyingrides_8791c8[(u32)angle & 0xfff], mul(out, 6)) / 1024);
            }
        }
        break;
    case 9:
        fly_on(index, k, record, slot);
        if (near_target(slot)) {
            state = 0;
            g_env->clearSlot(index, k);
        }
        break;
    default:
        break;
    }
}

// Each later slot in the air: close by, one slows and the lower climbs; closer in all three axes, the lower lifted and both rolled.
void keep_apart(const u8 *record, u8 *slot, u32 k) {
    for (u32 j = k; j < kSlots; j++) {
        u8 *const other = slot + (j - k) * kSlotSize;
        const s32 them = at<s32>(other, 0x80);
        if (j == k || them == 0 || them == 1) continue;
        const s32 width = at<s32>(record, 0x20);
        const s32 across = root(add(square(sub(at<s32>(slot, 4), at<s32>(other, 4)) / 32), square(sub(at<s32>(slot, 0xc), at<s32>(other, 0xc)) / 32)));
        if (mul(across, 32) < mul(width, 15000) / 1024) {
            const s32 dy = sub(at<s32>(other, 8), at<s32>(slot, 8));
            if ((dy < 0 ? (s32)(0u - (u32)dy) : dy) < mul(width, 3200) / 1024) {
                if (at<s32>(slot, 0x80) == 7) {
                    at<s32>(other, 0x74) = add(at<s32>(other, 0x74), 0x28);
                } else if (them == 7) {
                    at<s32>(slot, 0x74) = add(at<s32>(slot, 0x74), 0x28);
                } else {
                    const s32 mine = at<s32>(slot, 0x78), theirs = at<s32>(other, 0x78);
                    if (mine > theirs)
                        at<s32>(other, 0x78) = sub(theirs, theirs / 16);
                    else
                        at<s32>(slot, 0x78) = sub(mine, mine / 16);
                    if (at<s32>(slot, 8) > at<s32>(other, 8))
                        at<s32>(slot, 0x74) = add(at<s32>(slot, 0x74), 0x28);
                    else
                        at<s32>(other, 0x74) = add(at<s32>(other, 0x74), 0x28);
                }
            }
        }
        const s32 around = root(add(add(square(sub(at<s32>(slot, 4), at<s32>(other, 4)) / 32), square(sub(at<s32>(slot, 8), at<s32>(other, 8)) / 32)),
                                    square(sub(at<s32>(slot, 0xc), at<s32>(other, 0xc)) / 32)));
        if (mul(around, 32) < mul(at<s32>(record, 0x20), 3000) / 1024) {
            if (at<s32>(slot, 8) > at<s32>(other, 8))
                at<s32>(slot, 8) = add(at<s32>(slot, 8), 500);
            else
                at<s32>(other, 8) = add(at<s32>(other, 8), 500);
            at<s32>(slot, 0x18) = add(at<s32>(slot, 0x18), 1000);
            at<s32>(other, 0x18) = sub(at<s32>(other, 0x18), 1000);
        }
    }
}

void in_the_air(const u8 *record, u8 *slot, u32 k) {
    keep_apart(record, slot, k);
    s32 turn = sub(sub(0, at<s32>(slot, 0x10)), bearing(sub(at<s32>(slot, 0x54), at<s32>(slot, 0xc)), sub(at<s32>(slot, 0x4c), at<s32>(slot, 4)), -652.0)) & 0xfff;
    if (turn >= 0x800) turn -= 0xfff;  // sic
    turn = add(turn, at<s32>(slot, 0x70)) / 2;
    at<s32>(slot, 0x70) = turn;
    at<s32>(slot, 0x10) = add(turn / 10, at<s32>(slot, 0x10)) & 0xfff;
    if (at<s32>(slot, 0x80) == 7) {
        if (at<s32>(slot, 0x14) < 0x3c) at<s32>(slot, 0x14) += 2;
        const s32 glide = (s32)((u32)target_root(slot) << (at<u32>(record, 0x4c) != 0 ? 6 : 5));
        at<s32>(slot, 8) = add(add(glide, at<s32>(record, 0x18)), add(mul(at<s32>(slot, 8), 7), -3000)) / 8;
    } else {
        const s32 gap = sub(at<s32>(slot, 0x50), at<s32>(slot, 8));
        const s32 by = (gap < 0 ? (s32)(0u - (u32)gap) : gap) <= 10000 ? 1 : 3;
        s32 step = gap / 20;
        if (step > 0x10) step = 0x10;
        if (step < -0x10) step = -0x10;
        s32 climb = add(at<s32>(slot, 0x74), by * step);
        if (climb > 0x200) climb = 0x200;
        if (climb < -0x200) climb = -0x200;
        climb = mul(climb, 7) / 8;
        at<s32>(slot, 0x74) = climb;
        at<s32>(slot, 8) = add(at<s32>(slot, 8), mul(climb, 3));
        at<s32>(slot, 0x14) = mul(at<s32>(record, 0x24), climb) / 1024;
    }
    const s32 yaw = at<s32>(slot, 0x70);
    s32 roll = mul(sub(sub(0, yaw / 4), yaw / 2), at<s32>(record, 0x24)) / 1024 & 0xfff;
    const s32 now = at<s32>(slot, 0x18) & 0xfff;
    const s32 apart = roll - now;
    if (apart > 0x800)
        roll -= 0x1000;
    else if (apart < -0x800)
        roll += 0x1000;
    at<s32>(slot, 0x18) = (now * 7 + roll) / 8 & 0xfff;
}

void update_slot(s32 index, u32 k, u8 *record, u8 *slot) {
    for (u32 off = 0; off < 0x18; off += 4) at<u32>(slot, 0x1c + off) = at<u32>(slot, 4 + off);
    if (k > 0 && at<s32>(record, 0xc) > at<s32>(record, 0) && at<s32>(record, 0xc) > 1) at<u32>(slot, 0xb4) = 1;
    by_state(index, k, record, slot);
    // The speed: eased, then towards +0x7c; along the heading.
    const s32 eased = mul(at<s32>(slot, 0x78), at<s32>(slot, 0x80) == 1 ? 500 : 900) / 1024;
    at<s32>(slot, 0x70) = mul(at<s32>(slot, 0x70), 31) / 32;
    at<s32>(slot, 0x84) = sub(at<s32>(slot, 0x84), 1);
    const s32 goal = at<s32>(slot, 0x7c);
    const s32 speed = add(eased, add(mul(at<s32>(record, 0x50), goal) / 10, goal) / 10);
    at<s32>(slot, 0x78) = speed;
    at<s32>(slot, 0x64) = cosine(at<s32>(slot, 0x10), speed);
    at<s32>(slot, 0x6c) = sine(at<s32>(slot, 0x10), speed);
    at<s32>(slot, 4) = add(at<s32>(slot, 4), at<s32>(slot, 0x64));
    at<s32>(slot, 0xc) = add(at<s32>(slot, 0xc), at<s32>(slot, 0x6c));
    if (at<s32>(slot, 0x80) == 1) {
        if (at<s32>(slot, 0x14) > 0) at<s32>(slot, 0x14) -= 0x32;
    } else {
        in_the_air(record, slot, k);
    }
    if (const u32 sound = at<u32>(slot, 0xe0))
        at<u32>(slot, 0xe0) = g_env->setPosition(sound, at<s32>(slot, 4) / 300, at<s32>(slot, 8) / 300, at<s32>(slot, 0xc) / 300);
}

}  // namespace

void FlyingUpdate_SetEnv(const FlyingUpdateEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

void __cdecl FlyingRides_Update() {
    for (s32 index = 1; index < 100; index++) {
        u8 *const record = gFlyingrides_8791f8[(u32)index];
        if (!record) continue;
        if (at<s32>(record, 0xc) < at<s32>(record, 0)) g_env->launch(index);
        for (u32 k = 0; k < kSlots; k++) {
            u8 *const slot = record + 0x5c + k * kSlotSize;
            if (at<u32>(slot, 0x80) != 0) update_slot(index, k, record, slot);
        }
    }
}
}
