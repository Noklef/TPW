#include "objectstatus.h"

#include <cstdint>

#include "../coaster/registry.h"
#include "../core/accessors.h"
#include "../core/indexed.h"
#include "object.h"
#include "objectdefs.h"
#include "objectmsg.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The waiting statuses: none when the world's word is 0, busy when the game table entry's indexed dword is set, then by the tracked person:
// none, there, or there in its state.
u32 WaitStatus(u8 *object, u32 word, u32 index, u32 base) {
    u8 *const world = *reinterpret_cast<u8 **>(0x007cf83c);
    if (at<u16>(world, word) == 0)
        return base;
    if (Obj_GetIndexed28(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da718_519450(world)), nullptr, index) != 0)
        return base + 2;
    u16 handle;
    if (*Obj_GetTrackedHandle(object, nullptr, &handle) == 0)
        return base + 1;
    return Obj_TrackedPersonInState(object) ? base + 4 : base + 3;
}

}  // namespace

extern "C" {

u32 __cdecl Object_Status(u8 *object, s32 count) {
    const u8 *const def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(object, 0xe));
    if (Is19c_Eq4_4e0380(object))
        return 3;
    switch (at<u32>(def, 0x9c)) {
    case 1:
    case 2:
        if (at<u32>(object, 0x2c) == 0)
            return 0x19;
        break;
    case 3:
        if (!Object_LinkOpen(object))
            return 0x19;
        if (!Object_LinkBare(object))
            return 0x1a;
        break;
    }
    if (Is19c_Eq2_4e03e0(object))
        return WaitStatus(object, 0x1da73e, 1, 5);
    if (Get64_4e03d0(object))
        return at<u8>(object, 0x32) & 1 ? WaitStatus(object, 0x1da73c, 0, 0xb) : WaitStatus(object, 0x1da73e, 1, 0x11);
    if (Is19c_Eq1_4e0370(object))
        return Obj_TrackedPersonInState(object) ? 0x15 : 2;
    if (at<u8>(object, 0x32) & 8 && !Object_EntranceOnPath(object))
        return Is68_Zero_4e0440(object) ? 0x17 : 0x16;
    if (at<s32>(def, 0x4ac) == 0) {
        u16 word;
        if (*Object_DefWord284(object, nullptr, &word) == 0 && at<s32>(def, 0x288) != 2 && !Object_ExitOpen(object))
            return 0x18;
    }
    if (count == 0)
        return 1;
    return count > 1 && Is68_Zero_4e0440(object) ? 1 : 0;
}
}
