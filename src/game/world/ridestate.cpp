#include "ridestate.h"

#include <cstdint>

#include "../coaster/registry.h"
#include "../core/misc.h"
#include "../model/holder.h"
#include "../model/parts.h"
#include "../model/partstate.h"
#include "flyingrides.h"
#include "ridelist.h"
#include "trackrides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __cdecl Ride_IsBusy(const u8 *object, u32 index) {
    if (object) {
        const u32 handle = at<u32>(object, 0x28);
        if (handle && TrakRide_LinkedValue(handle)) return 1;
        const u32 flying = Rides_Get9c(Rides_FindById(at<s32>(object, 0x24)));
        if (flying && FlyingRide_FirstSlotRide((s32)flying)) return 1;
    }
    if (Ride_Bit18(index) && CoasterLink_IdValue(index)) return 1;
    return Model_FindPartByKey4(reinterpret_cast<const u8 *const *>(0x007a4610)[index], 0x1000, 1) != 0xffffffff;
}

void __cdecl Ride_Sleep(u32 index) {
    u8 *const holder = reinterpret_cast<u8 *const *>(0x007a4610)[index];
    if (!holder) return;
    u8 *const instance = at<u8 *>(holder, 8);
    if (at<u32>(instance, 0) & 1) {
        if (at<u32>(holder, 4) & 0x4000)
            Obj_RemoveFromRect(instance, at<u32>(holder, 0xc0), at<u32>(holder, 0xc4));
        else
            Holder_Sleep(instance);
    }
    for (u32 nth = 1; nth < 0x100; ++nth) {
        const u32 part = Model_NthPart(holder, 0x80, nth);
        if (part == 0xffffffff) continue;
        u8 *const states = at<u8 *>(holder, 8);
        if (at<u32>(at<u8 *>(at<u8 *>(states, 0x28), 4), part * 0x14) & 2) PartStates_Release(states, part);
    }
}
}
