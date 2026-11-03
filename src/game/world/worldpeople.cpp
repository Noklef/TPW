#include "worldpeople.h"

#include <cstdint>

#include "../core/funnytime.h"
#include "objectunlock.h"
#include "parkgates.h"
#include "staffcandidate.h"

namespace {

const WorldPeopleEnv kGameEnv = {&StaffPool_Start, &FunnyTime_SyncMonthYear, &ParkGates_Step, &StaffPool_Update, &FunnyTime_Update, &ObjectUnlock_Step};
const WorldPeopleEnv *g_env = &kGameEnv;

}  // namespace

void WorldPeople_SetEnv(const WorldPeopleEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall WorldPeople_Start(u8 *world) {
    g_env->staffStart(world);
    g_env->clockSync(world + 0x2a0);
}

void __fastcall WorldPeople_Step(u8 *world) {
    g_env->gatesStep(world + 0x2c4);
    g_env->staffUpdate(world);
    g_env->clockUpdate(world + 0x2a0);
    const u8 *const game = **reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec);
    if (*reinterpret_cast<const u32 *>(game + 0x1da70c) % 100 == 0) g_env->unlockStep(world + 0x2c0);
}
}
