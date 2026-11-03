#include "staffwork.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../sprite/sprite.h"
#include "advisor.h"
#include "advisorscore.h"
#include "litter.h"
#include "object.h"
#include "person.h"
#include "random.h"
#include "staff.h"
#include "thingscan.h"
#include "world.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

u32(__fastcall *const kBusy)(u8 *) = reinterpret_cast<u32(__fastcall *)(u8 *)>(0x00506a40);
u32(__fastcall *const kRandomDest)(u8 *) = reinterpret_cast<u32(__fastcall *)(u8 *)>(0x004f9490);

const StaffWorkEnv kGameEnv = {kBusy, &Staff_IsTooTired, kRandomDest, &World_NearestCellWithType, &Handyman_FindJob, &Sprite_SetDestination, &Obj_StampAt30, &Handyman_SetState, &Objects_NearestFreeFor, &Sprite_SetDestinationCell, &Obj_StampAt5e, &Mechanic_SetState, &Staff_SetState, &Things_AnyOfKindAround, &Sprite_ActionCount};
const StaffWorkEnv *g_env = &kGameEnv;

u8 *pointerOf(u32 record) { return reinterpret_cast<u8 *>((uintptr_t)record); }

u32 pointInCell(u32 cellPart, u8 *world) {
    const u32 draw = static_cast<u32>(World_Rand(world)) & 0x7f;
    const u32 clamped = draw < 1 ? 1 : draw > 9 ? 9 : draw;
    return (cellPart & 0xff) << 8 | clamped * 13;
}
}  // namespace

void StaffWork_SetEnv(const StaffWorkEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall Handyman_PickWork(u8 *handyman) {
    if (g_env->busy(handyman)) return;
    if (!g_env->tooTired(handyman)) {
        u8 *const world = global<u8 *>(0x0080239c);
        const u16 handle = at<u16>(handyman, 0);
        const u32 cell = ((u32)handyman[7] << 7) + handyman[5] + 1;
        u16 out;
        const u16 target = *g_env->nearestCell(pointerOf(GameTable7cfb90_Get1da720_519510(world)), nullptr, &out, handle, cell, global<s32>(0x007853f0 + at<u32>(handyman, 0x1e4) * 12));
        at<u16>(handyman, 0x218) = target;
        if (target) {
            World_SetSeed(world, nullptr, target);
            const u32 index = (u32)target - 1;
            const s32 x = static_cast<s32>(pointInCell(index & 0x7f, world));
            const s32 y = static_cast<s32>(pointInCell(index >> 7, world));
            if (g_env->setDestination(handyman, nullptr, x, y) == 1) {
                g_env->stampCell(global<u8 *>(0x008023a0) + (u32)target * 0x44 - 0x44, nullptr, handle);
                g_env->handymanState(handyman, nullptr, 8);
                return;
            }
        } else {
            const u16 job = *g_env->findJob(handyman, nullptr, &out);
            at<u16>(handyman, 0x21a) = job;
            if (job) {
                g_env->handymanState(handyman, nullptr, 0xa);
                return;
            }
        }
    }
    g_env->handymanState(handyman, nullptr, g_env->randomDest(handyman) ? 1 : 0);
}

void __fastcall Mechanic_PickWork(u8 *mechanic) {
    if (g_env->busy(mechanic)) return;
    if (!g_env->tooTired(mechanic)) {
        u8 *const world = global<u8 *>(0x0080239c);
        const u16 handle = at<u16>(mechanic, 0);
        u16 out;
        const u16 target = *g_env->nearestFree(pointerOf(GameTable7cfb90_Get1da71a_519490(world)), nullptr, &out, handle);
        at<u16>(mechanic, 0x218) = target;
        if (target) {
            u8 *const object = global<u8 *>(0x007cfb90 + (u32)target * 20);
            if (g_env->setDestinationCell(mechanic, nullptr, at<u16>(object, 0x36))) {
                g_env->stampObject(object, nullptr, handle);
                if (!at<u32>(object, 0x64) && at<u32>(object, 0x19c) != 2) {
                    u8 message[0x18];
                    AdvisorMsg_Init(message, nullptr, 0x46, target, at<u16>(object, 0xe), 0xffffffffu, 0, -1);
                    Advisor_Post(pointerOf(GameTable7cfb90_Get1da71c_5194d0(world)), nullptr, message);
                }
                g_env->mechanicState(mechanic, nullptr, 0xc);
                return;
            }
        }
    }
    g_env->mechanicState(mechanic, nullptr, g_env->randomDest(mechanic) ? 1 : 0);
}

void __fastcall Entertainer_PickWork(u8 *entertainer) {
    if (g_env->busy(entertainer)) return;
    u8 *const world = global<u8 *>(0x0080239c);
    if (!g_env->tooTired(entertainer) && static_cast<u32>(World_Rand(world)) % 3 == 0) {
        const u32 cell = ((u32)entertainer[7] << 7) + entertainer[5] + 1;
        if (g_env->anyAround(1, cell, global<s32>(0x007853a0 + (at<u32>(entertainer, 0x1e4) << 4)))) {
            const u32 draw = static_cast<u32>(World_Rand(world));
            const u32 actions = g_env->actionCount(at<u32>(entertainer, 0x24), at<u32>(entertainer, 0x20));
            Set4_4217f0(entertainer + 0xc, nullptr, actions == 1 ? 0xd : draw % (actions - 1) + 0xd);
            at<u32>(entertainer, 0x19c) = 0xe;
            at<u32>(entertainer, 0x214) = at<u32>(world, 0x1da70c);
            return;
        }
    }
    if (at<u32>(world, 0x1da70c) & 3) {
        if (!g_env->randomDest(entertainer)) return;
        World_Rand(world);
        g_env->staffState(entertainer, nullptr, 1);
        at<u32>(entertainer, 0x19c) = 1;
    } else {
        World_Rand(world);
        g_env->staffState(entertainer, nullptr, 0);
        at<u32>(entertainer, 0x19c) = 0;
    }
}
}
