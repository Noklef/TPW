// An object's ride: see objectride.h.
#include "objectride.h"

#include <cstdint>

#include "../coaster/queue.h"
#include "../coaster/registry.h"
#include "objectdefs.h"
#include "ridelist.h"
#include "rides.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const ObjectRideEnv kGameEnv = {ObjectDefs_Find, reinterpret_cast<u32(__cdecl *)(u32, u32, u32, const u32 *, u32, u32, u32, u32, u32)>(Ride_Claim),
                                Ride_Create, Ride_SetRecordId, Coaster_RequeueWaitingPeeps};
const ObjectRideEnv *g_env = &kGameEnv;

}  // namespace

void ObjectRide_SetEnv(const ObjectRideEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall Object_ClaimRide(u8 *object, void *, u32 claim) {
    const u8 *const def = g_env->find(reinterpret_cast<u8 *>((uintptr_t)0x007890a0), nullptr, at<u16>(object, 0xe));
    if (!def) return;
    u32 desc[4];
    desc[2] = at<u32>(object, 0x10);
    desc[3] = at<u32>(def, 0x18);
    const u32 self = (u32)(uintptr_t)object;
    if (at<u32>(def, 0x38)) {
        desc[0] = at<u32>(def, 0x20) ? at<u32>(def, 0x20) : global<u32>((u32)(uintptr_t)&gBalanceLevel + 0x554);
        desc[1] = at<u32>(def, 0x24) ? at<u32>(def, 0x24) : global<u32>((u32)(uintptr_t)&gBalanceLevel + 0x558);
        if (claim) {
            g_env->claim(at<u32>(object, 0x20), at<u32>(def, 0x490), 0x361, desc, 0, self, at<u32>(def, 0x28), at<u32>(def, 0x2c), at<u32>(object, 0x24));
            return;
        }
        at<u32>(object, 0x20) = g_env->create(at<u32>(def, 0x490), 0x361, desc, 0, at<u32>(object, 0x24), self, at<u32>(def, 0x28), at<u32>(def, 0x2c), -1, 0);
        return;
    }
    const u32 cell = (u32)at<u16>(object, 0x34) - 1;
    desc[0] = cell & 0x7f;
    desc[1] = cell >> 7;
    if (claim) {
        g_env->claim(at<u32>(object, 0x20), at<u32>(def, 0x490), 0x32f, desc, 0, self, 0, 0, 0);
        g_env->setRecordId(at<u32>(object, 0x20), at<u32>(object, 0x24));
        g_env->requeue(at<s32>(object, 0x20));
        return;
    }
    at<u32>(object, 0x20) = g_env->create(at<u32>(def, 0x490), 0x32f, desc, 0, at<u32>(object, 0x24), self, 0, 0, -1, 0);
}
}
