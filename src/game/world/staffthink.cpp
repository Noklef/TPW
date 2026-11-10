// A staff member's step by its state: see staffthink.h.
#include "staffthink.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/soundplay.h"
#include "../sprite/script.h"
#include "guardescort.h"
#include "mapcell.h"
#include "object.h"
#include "person.h"
#include "random.h"
#include "staff.h"
#include "staffsteps.h"
#include "staffwork.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename F> F raw(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }
using Step = void(__fastcall *)(u8 *);

const StaffThinkEnv kGameEnv = {World_Rand, Sprite_PlaySound, Sprite_WalkNav, Staff_TireA, Staff_TireB, Staff_WalkToRestArea, raw<Step>(0x005061d0), raw<Step>(0x005056e0), raw<u32(__fastcall *)(u8 *)>(0x00506a40), &Person_SetRandomDest,
                               Handyman_PickWork, Handyman_WalkToLitter, Handyman_SweepLitter, Handyman_WalkToJob, Object_ResetCondition, Object_Close, Set4_4217f0, Handyman_SetState, Guard_SetState, Guard_Chase, Guard_LeaveWithVisitor, Map_EntrancePos, Sprite_SetDestinationCell,
                               Entertainer_PickWork, SpriteScript_GetPosition, Sound_PlayAt, Mechanic_PickWork, Mechanic_WalkToObject, Mechanic_Repair};
const StaffThinkEnv *g_env = &kGameEnv;

u8 *world() { return global<u8 *>(0x0080239c); }
u32 tick() { return at<u32>(world(), 0x1da70c); }

void sometimesSound(u8 *staff, u32 sound) {
    if (!(g_env->rand(world()) & 0xf)) g_env->playSound(staff, nullptr, global<u32>(0x00803a30), sound);
}

// The walk: true on arrival (0 or 2); else the walker tires.
bool arrived(u8 *staff, u32 sound) {
    sometimesSound(staff, sound);
    const u32 walked = g_env->walk(staff);
    if (walked == 0 || walked == 2) return true;
    g_env->tireA(staff);
    return false;
}

// The rest over: true past the level's wait (from +0x200).
bool restOver(u8 *staff, u32 sound) {
    sometimesSound(staff, sound);
    return tick() > global<u32>(((u32)(uintptr_t)&gBalanceLevel + 0x2f0) + (at<u32>(staff, 0x1e4) << 4)) + at<u32>(staff, 0x200);
}

void guardWork(u8 *guard) {
    if (g_env->busy(guard)) return;
    g_env->guardState(guard, nullptr, (at<u8>(world(), 0x1da70c) & 3) && g_env->patrol(guard) ? 1 : 0);
}

}  // namespace

void StaffThink_SetEnv(const StaffThinkEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall Guard_PickWork(u8 *guard) { guardWork(guard); }

void __fastcall Guard_Step(u8 *guard) {
    switch (at<u32>(guard, 0x19c)) {
    case 0:
        if (restOver(guard, 0xa7)) guardWork(guard);
        break;
    case 1:
        if (arrived(guard, 0xa6)) guardWork(guard);
        break;
    case 2: g_env->toRest(guard); break;
    case 3: g_env->atRest(guard); break;
    case 0x10: g_env->chase(guard); break;
    case 0x12: g_env->leave(guard); break;
    case 0x13: {
        const u32 walked = g_env->walk(guard);
        if (walked != 0 && walked != 2) break;
        u16 out;
        const u16 cell = *g_env->entrance(global<u8 *>(0x008023a0), nullptr, &out, tick() & 1);
        g_env->guardState(guard, nullptr, g_env->destination(guard, nullptr, cell) ? 1 : 0);
        break;
    }
    default: g_env->other(guard); break;
    }
}

void __fastcall Handyman_Step(u8 *handyman) {
    switch (at<u32>(handyman, 0x19c)) {
    case 0:
        if (restOver(handyman, 0xa1)) g_env->handymanWork(handyman);
        break;
    case 1:
        if (arrived(handyman, 0xa0)) g_env->handymanWork(handyman);
        break;
    case 2: g_env->toRest(handyman); break;
    case 3: g_env->atRest(handyman); break;
    case 8: g_env->toLitter(handyman); break;
    case 9: g_env->sweep(handyman); break;
    case 0xa: g_env->toJob(handyman); break;
    case 0xb:
        g_env->tireB(handyman);
        if (tick() > global<u32>(((u32)(uintptr_t)&gBalanceLevel + 0x3ac) + at<u32>(handyman, 0x1e4) * 12) + at<u32>(handyman, 0x214)) {
            u8 *const object = global<u8 *>(0x007cfb90 + at<u16>(handyman, 0x21a) * 20u);
            g_env->resetCondition(object);
            g_env->close(object);
            g_env->set4(handyman + 0xc, nullptr, 3);
            g_env->handymanState(handyman, nullptr, 0);
        }
        break;
    default: g_env->other(handyman); break;
    }
}

void __fastcall Entertainer_Step(u8 *entertainer) {
    switch (at<u32>(entertainer, 0x19c)) {
    case 0:
        if (restOver(entertainer, 0xa5)) g_env->entertainerWork(entertainer);
        break;
    case 1:
        if (arrived(entertainer, 0xa4)) g_env->entertainerWork(entertainer);
        break;
    case 2: g_env->toRest(entertainer); break;
    case 3: g_env->atRest(entertainer); break;
    case 0xe:
        g_env->tireB(entertainer);
        if (tick() > global<u32>(((u32)(uintptr_t)&gBalanceLevel + 0x358) + (at<u32>(entertainer, 0x1e4) << 4)) + at<u32>(entertainer, 0x214)) {
            float x, y, z;
            u32 r15;
            g_env->position(at<s32>(entertainer, 0xc), reinterpret_cast<u32 *>(&x), reinterpret_cast<u32 *>(&y), reinterpret_cast<u32 *>(&z), &r15);
            const s32 zAt = (s32)(long long)z, yAt = (s32)(long long)y, xAt = (s32)(long long)x;
            g_env->playAt(0, global<u32>(0x00803a30), 0x87, xAt, yAt, zAt);
            g_env->entertainerWork(entertainer);
        }
        break;
    default: g_env->other(entertainer); break;
    }
}

void __fastcall Mechanic_Step(u8 *mechanic) {
    switch (at<u32>(mechanic, 0x19c)) {
    case 0:
        if (restOver(mechanic, 0xa3)) g_env->mechanicWork(mechanic);
        break;
    case 1:
        if (arrived(mechanic, 0xa2)) g_env->mechanicWork(mechanic);
        break;
    case 2: g_env->toRest(mechanic); break;
    case 3: g_env->atRest(mechanic); break;
    case 0xc: g_env->toObject(mechanic); break;
    case 0xd: g_env->repair(mechanic); break;
    default: g_env->other(mechanic); break;
    }
}
}
