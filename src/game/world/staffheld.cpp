// A staff member held by a building and the world's staff slots: see staffheld.h.
#include "staffheld.h"

#include <cstdint>

#include "../ui/menuactions.h"
#include "staff.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

void __fastcall staffChanged(u32, void *, u32 index) { Wrap_Dialog7ca308_StaffChanged_481550(index); }

const StaffHeldEnv kGameEnv = {Staff_PlaceInCell, staffChanged};
const StaffHeldEnv *g_env = &kGameEnv;

}  // namespace

void StaffHeld_SetEnv(const StaffHeldEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall Holder_PlaceStaff(u8 *holder, void *, u32 cell) {
    g_env->place(global<u8 *>(0x007cfb90 + at<u16>(holder, 0x60) * 0x14), nullptr, cell);
    at<u16>(holder, 0x60) = 0;
}

void __fastcall World_ClearStaffSlots(u8 *world, void *, u32 keep) {
    for (u32 i = 0; i < 0x20; i++) {
        u8 *const slot = world + 0xa + i * 0x14;
        if (!slot[1]) continue;
        if (!keep) {
            slot[0] = 0;
            g_env->changed(0x007b51f0, nullptr, i);
        }
        slot[1] = 0;
    }
}
