#include "ridestate.h"

#include <cstdint>

#include "../coaster/registry.h"
#include "../model/parts.h"
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
}
