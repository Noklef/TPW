#include "flyinglaunch.h"

#include <cstdint>

#include "../core/soundplay.h"
#include "../files/file.h"
#include "../model/parts.h"
#include "flyingrides.h"
#include "objectdefs.h"
#include "random.h"
#include "rides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const FlyingLaunchEnv kPortEnv = {&Model_FindPartByKey4, &Ride_Create, &Sound_PlayRideElement, &FlyingRide_PlaySlotMotion, &FlyingRide_Launch};
const FlyingLaunchEnv *g_env = &kPortEnv;

const u32 kSlots = 20, kSlotSize = 0xe4;
const s32 kOff = 0xafc8;

// The magnitude of a World_Rand (wrapping).
s32 Rand() {
    const s32 r = World_Rand(global<u8 *>(0x007cf83c));
    return r < 0 ? (s32)(0u - (u32)r) : r;
}

// A random place between the two (100 * random mod the span, from the least).
s32 Inside(s32 high, s32 low) {
    const s32 hundred = (s32)((u32)Rand() * 100);
    return (s32)((u32)(hundred % (s32)((u32)high - (u32)low)) + (u32)low);
}

// One of the two sides, past them by kOff.
s32 Beyond(s32 high, s32 low) { return (Rand() & 1) ? (s32)((u32)high + (u32)kOff) : (s32)((u32)low - (u32)kOff); }

}  // namespace

void FlyingLaunch_SetEnv(const FlyingLaunchEnv *env) { g_env = env ? env : &kPortEnv; }

extern "C" {

u32 __cdecl FlyingRide_Launch(s32 index) {
    u8 *const record = global<u8 *>(0x008791f8 + (u32)index * 4);
    u32 k = 0;
    u8 *slot = record + 0x5c;
    while (at<u32>(slot, 0x80) != 0) {
        if (++k >= kSlots) return 0;
        slot += kSlotSize;
    }
    at<u32>(slot, 0xe0) = 0;
    at<s32>(slot, 0) = index;
    at<u32>(slot, 0x18) = 0;
    at<u32>(slot, 0x14) = 0;
    at<u32>(slot, 0x74) = 0;
    at<u32>(slot, 0x70) = 0;
    at<u32>(slot, 0x78) = 0;
    const s32 wait = Rand() % 100;
    at<u32>(slot, 0x64) = 0;
    at<u32>(slot, 0x68) = 0;
    at<u32>(slot, 0x6c) = 0;
    at<u32>(slot, 0x80) = 4;
    at<u32>(slot, 0xb8) = 0;
    at<u32>(slot, 0xb4) = 0;
    at<u32>(slot, 0x8c) = 0;
    at<u32>(slot, 0xc4) = 0;
    at<u32>(slot, 0xc8) = 1;
    at<u32>(slot, 0xc0) = 0;
    at<u32>(slot, 0x50) = 0x36b0;
    at<u32>(slot, 0x7c) = (u32)wait + at<u32>(record, 0x28);

    const s32 xHigh = global<s32>(0x007660b8), xLow = global<s32>(0x007660bc), zHigh = global<s32>(0x007660c0), zLow = global<s32>(0x007660c4);
    s32 x, z;
    if (Rand() & 1) {
        x = Inside(xHigh, xLow);
        z = Beyond(zHigh, zLow);
    } else {
        z = Inside(zHigh, zLow);
        x = Beyond(xHigh, xLow);
    }
    at<s32>(slot, 0x4c) = x;
    at<s32>(slot, 0x54) = z;
    const s32 y = at<s32>(slot, 0x50);
    at<s32>(slot, 4) = x;
    at<s32>(slot, 8) = y;
    at<s32>(slot, 0xc) = z;
    at<u32>(slot, 0x84) = 0;
    at<u32>(slot, 0x58) = (u32)x - 0x4e20;
    at<s32>(slot, 0x5c) = y;
    at<s32>(slot, 0x60) = z;
    ++at<u32>(record, 0xc);
    at<s32>(slot, 0xcc) = -1;
    at<s32>(slot, 0xd0) = -1;
    at<s32>(slot, 0xd4) = -1;
    at<u32>(slot, 0xe0) = g_env->playSound(at<s32>(record, 0x54), 0, global<u32>(0x00803a3c), 0, x / 300, y / 300, z / 300);
    at<s32>(slot, 0xd8) = Rand();

    if (at<u32>(record, 0xc) == 1) {
        at<u32>(slot, 0x80) = 1;
        at<u32>(slot, 4) = at<u32>(record, 0x14);
        at<u32>(slot, 8) = at<u32>(record, 0x18);
        at<u32>(slot, 0xc) = at<u32>(record, 0x1c);
        at<u32>(slot, 0x7c) = 0;
        at<u32>(slot, 0x10) = at<u32>(record, 0x40);
        at<u32>(record, 0x3c) = 1;
    }

    const u8 *const object = global<const u8 *>(0x007cfb90 + (u32)at<u16>(record, 0x58) * 0x14);
    const u8 *const def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(object, 0xe));
    at<u32>(slot, 0xc0) = g_env->createRide(at<u32>(def, 0x4bc), 0x101, nullptr, 0, 0, 0, 0, 0, -1, 0);
    g_env->playMotion(index, k, 0, 0, 0);

    u32 parts = 0;
    while (g_env->findPart(global<const u8 *>(0x007a4610 + at<u32>(slot, 0xc0) * 4), 0x80, parts + 1) != 0xffffffff) ++parts;
    at<u32>(slot, 0xdc) = parts;
    return 1;
}

s32 __cdecl FlyingRide_Create(u32 object, u32 sound, u32 x, u32 y, u32 z, u32 turn, u32 quick) {
    s32 index = 1;
    while (index < 100 && global<u32>(0x008791f8 + index * 4) != 0) ++index;
    if (index >= 100) return -1;
    u8 *const record = static_cast<u8 *>(Mem_GlobalAlloc(0x122c));
    global<u8 *>(0x008791f8 + index * 4) = record;
    if (record) {
        at<u32>(record, 0x14) = x;
        at<u32>(record, 0x18) = y;
        at<u32>(record, 0x1c) = z;
        at<u32>(record, 0x20) = 0x400;
        at<u32>(record, 0x24) = 0x400;
        at<u32>(record, 0x40) = 0x400 - turn;
        at<u16>(record, 0x58) = (u16)object;
        at<u32>(record, 0x4c) = quick;
        at<u32>(record, 0x54) = sound;
        at<u32>(record, 0) = 1;
        at<u32>(record, 4) = 1;
        at<u32>(record, 0xc) = 0;
        at<u32>(record, 0x10) = 300;
        at<u32>(record, 0x3c) = 0;
        at<u32>(record, 0x44) = 0;
        at<u32>(record, 0x2c) = 1;
        at<u32>(record, 0x38) = 1;
        at<u32>(record, 0x48) = 0;
        for (u32 slot = 0; slot < 20; ++slot) at<u32>(record, 0xdc + slot * 0xe4) = 0;
        at<u32>(record, 0x28) = 300;
        at<u32>(record, 0x50) = 5;
        g_env->launch(index);
    }
    at<u32>(record, 0x30) = at<u32>(record, 0x4c) != 0 ? 0x9c4 : 0x1388;
    at<u32>(record, 0x34) = 1;
    return index;
}
}
