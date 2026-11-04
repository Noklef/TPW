#include "rideanimate.h"

#include <cstdint>

#include "../core/addrcalls.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

AddrCalls g_calls;

u32 Address(const void *p) { return (u32)(uintptr_t)p; }

void AnimatorUpdate(u8 *ride, u32 shown, u32 mode) { g_calls.Cd(0x00473c70, Address(ride), shown, mode); }

void AimSprites(u8 *ride) {
    if (at<u32>(at<u8 *>(ride, 8), 0x28)) g_calls.Cd(0x0044b510, Address(ride + 8));
}

void StripAnim(u8 *ride) {
    if (at<u8>(ride, 4) & 0x20) g_calls.Cd(0x004548f0, Address(ride), global<u32>(0x007b497c));
}

}  // namespace

void RideAnimate_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

void __cdecl Ride_AnimateOne(u8 *ride, u32 shown) {
    if (shown && (at<u32>(ride, 4) & 0x800000)) {
        if (!(at<u32>(ride, 4) & 0x80000)) g_calls.Cd(0x004726d0, Address(ride));
        at<u32>(ride, 4) &= 0xff7fffffu;
    }
    if (at<u32>(ride, 0x14) && !(at<u32>(ride, 4) & 0x3000)) {
        AnimatorUpdate(ride, shown, shown ? 0 : 8);
        if (shown) AimSprites(ride);
    }
    StripAnim(ride);
}

void __cdecl Rides_AnimateAll(u32 mode) {
    if (mode == 2) {
        if (!(global<u8>(0x00790ab0) & 4)) return;
        AnimatorUpdate(global<u8 *>(0x00790988), 1, 0);
        AimSprites(global<u8 *>(0x00790988));
        StripAnim(global<u8 *>(0x00790988));
        return;
    }
    for (u32 i = 0; i < global<u32>(0x007a3ebc) + global<u32>(0x007a4084); i++) {
        u8 *const ride = global<u8 *>(0x007a4614 + i * 4);
        if (!ride) continue;
        if (at<u32>(ride, 4) & 0x200000) {
            at<u32>(ride, 4) &= 0xffdfffffu;
            continue;
        }
        if (at<u32>(ride, 0x14) && !(at<u32>(ride, 4) & 0x3000)) AnimatorUpdate(ride, 0, 8);
        StripAnim(ride);
        at<u32>(ride, 4) |= 0x800000;
    }
}
}
