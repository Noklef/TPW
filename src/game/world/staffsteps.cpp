// The handyman's and mechanic's update and work states: see staffsteps.h.
#include "staffsteps.h"

#include <cstdint>

#include "../core/accessors.h"
#include "litter.h"
#include "object.h"
#include "person.h"
#include "personscripts.h"
#include "ridelist.h"
#include "staff.h"
#include "staffwork.h"
#include "things.h"
#include "thingscan.h"
#include "world.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

const StaffStepsEnv kGameEnv = {&Staff_TireB, &Sprite_WalkNav, &Thing_PlaceAt, &Handyman_SetState, &Litter_Remove, &Handyman_PickWork, &Mechanic_PickWork, &Mechanic_SetState, &Obj_ClearRideElement4, &Obj_RideElement7Positive, &Object_MechanicDone, &Person_TickSpeed, &Person_KillScript84After, &World_NearestCellWithType, &Obj_GetTrackedHandle};
const StaffStepsEnv *g_env = &kGameEnv;

u8 *litterOf(u32 number) { return global<u8 *>(0x008023a0) + number * 0x44 - 0x44; }
u8 *objectOf(u8 *mechanic) { return global<u8 *>(0x007cfb90 + at<u16>(mechanic, 0x218) * 20); }
u32 ticks() { return global<u32>((u32)(uintptr_t)global<u8 *>(0x0080239c) + 0x1da70c); }

void handymanNext(u8 *handyman) {
    at<u16>(handyman, 0x218) = 0;
    g_env->handymanWork(handyman);
}

void mechanicNext(u8 *mechanic) {
    at<u16>(mechanic, 0x218) = 0;
    g_env->mechanicWork(mechanic);
}

}  // namespace

void StaffSteps_SetEnv(const StaffStepsEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall Handyman_Update(u8 *handyman) {
    g_env->tick(handyman);
    if (at<u32>(handyman, 0x19c) == 0) {
        const u32 type = at<u32>(handyman, 0x1e4);
        if (ticks() % global<u32>(0x00785330 + type * 16) == 0) {
            u8 *const world = global<u8 *>(0x0080239c);
            const u32 cell = ((u32)handyman[7] << 7) + handyman[5] + 1;
            u16 out;
            if (*g_env->nearestCell(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(world)), nullptr, &out, at<u16>(handyman, 0), (u16)cell, global<s32>(0x007853f0 + type * 12))) g_env->handymanWork(handyman);
        }
    }
    g_env->killAfter(handyman + 0x30);
}

void __fastcall Handyman_WalkToLitter(u8 *handyman) {
    u8 *const litter = litterOf(at<u16>(handyman, 0x218));
    if (at<u16>(litter, 0x30) != at<u16>(handyman, 0)) {
        handymanNext(handyman);
        return;
    }
    const u32 walked = g_env->walkNav(handyman);
    if (walked == 0) {
        if (at<u32>(litterOf(at<u16>(handyman, 0x218)), 0x2c) == 0) {
            handymanNext(handyman);
            return;
        }
        g_env->placeAt(handyman, nullptr, at<u16>(handyman, 0x18), at<u16>(handyman, 0x1a));
        g_env->handymanState(handyman, nullptr, 9);
    } else if (walked == 2) {
        handymanNext(handyman);
    }
}

void __fastcall Handyman_SweepLitter(u8 *handyman) {
    g_env->tire(handyman);
    if (ticks() <= global<u32>(0x007853ec + at<u32>(handyman, 0x1e4) * 12) + at<u32>(handyman, 0x214)) return;
    if (at<u32>(litterOf((u16)(((u32)handyman[7] << 7) + handyman[5] + 1)), 0x2c)) g_env->removeLitter(litterOf(at<u16>(handyman, 0x218)));
    handymanNext(handyman);
}

void __fastcall Mechanic_WalkToObject(u8 *mechanic) {
    u8 *const object = objectOf(mechanic);
    u16 out;
    if (*g_env->tracked(object, nullptr, &out) != at<u16>(mechanic, 0)) {
        mechanicNext(mechanic);
        return;
    }
    const u32 walked = g_env->walkNav(mechanic);
    if (walked == 0) {
        if (Is19c_Eq1_4e0370(object) || Is19c_Eq2_4e03e0(object) || Get64_4e03d0(object)) g_env->mechanicState(mechanic, nullptr, 0xd);
        else g_env->mechanicWork(mechanic);
    } else if (walked == 2) {
        mechanicNext(mechanic);
    }
}

void __fastcall Mechanic_Repair(u8 *mechanic) {
    g_env->tire(mechanic);
    u8 *const object = objectOf(mechanic);
    const u32 count = at<u32>(mechanic, 0x214);
    if (count) {
        at<u32>(mechanic, 0x214) = count - 1;
        return;
    }
    if (Is19c_Eq1_4e0370(object)) {
        g_env->clearElement(object);
        if (g_env->elementPositive(object)) return;
    }
    g_env->mechanicDone(object, nullptr, at<u16>(mechanic, 0));
    mechanicNext(mechanic);
}
