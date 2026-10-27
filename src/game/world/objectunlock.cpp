#include "objectunlock.h"

#include <cstdint>

#include "../core/globalsave.h"
#include "../core/stdtree.h"
#include "advisorscore.h"
#include "objectdefs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 __stdcall ObjectDef_IsUnlocked(u32 id) {
    const s32 needed = at<s32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, (u16)id), 0xc4);
    return needed <= GlobalSave_CountSetLeft(GlobalSave_Instance()) ? 1 : 0;
}

u8 __stdcall ObjectDef_Unlock(u32 id) {
    const s32 needed = at<s32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, (u16)id), 0xc4);
    if (ModeSetting_Read(0x00fb34ba) == 1) return 1;
    if (!ObjectDef_IsUnlocked(id)) return 0;
    return Object_AddKeyed(GlobalSave_Instance(), nullptr, id, needed);
}
}
