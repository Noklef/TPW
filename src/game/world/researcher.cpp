// A researcher's tick: see researcher.h.
#include "researcher.h"

#include <cstdint>

#include "../core/idmaps.h"
#include "person.h"
#include "research.h"
#include "world.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

const ResearcherEnv kGameEnv = {Mode19c_ToIndex, GameTable7cfb90_Get1da722_519550, ResearchLab_Spend, Person_TickSpeed};
const ResearcherEnv *g_env = &kGameEnv;

}  // namespace

void Researcher_SetEnv(const ResearcherEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall Researcher_Update(u8 *person) {
    const u32 mode = g_env->mode(person);
    if (mode != 3 && mode != 4 && mode != 5 && at<u32>(global<u8 *>(0x0080239c), 0x1da70c) % 20 == 0) {
        const s32 wage = global<s32>(((u32)(uintptr_t)&gBalanceLevel + 0x41c) + at<u32>(person, 0x1e4) * 12);
        u8 *const lab = reinterpret_cast<u8 *>((uintptr_t)g_env->lab(global<u8 *>(0x0080239c)));
        g_env->spend(lab, nullptr, wage);
    }
    g_env->tick(person);
}
