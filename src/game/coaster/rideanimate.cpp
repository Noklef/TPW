#include "rideanimate.h"

#include <cstdint>

#include "../core/addrcalls.h"
#include "../world/rides.h"
#include "../world/camera.h"

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
    if (at<u8>(ride, 4) & 0x20) g_calls.Cd(0x004548f0, Address(ride), reinterpret_cast<u32 &>(gMotionTimeStep));
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
        if (!(reinterpret_cast<u8 &>(gCameraFlags) & 4)) return;
        AnimatorUpdate(gCamera_790988, 1, 0);
        AimSprites(gCamera_790988);
        StripAnim(gCamera_790988);
        return;
    }
    for (u32 i = 0; i < global<u32>(0x007a3ebc) + global<u32>(0x007a4084); i++) {
        u8 *const ride = gRides_7a4610[i + 1];
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

void __cdecl Object_AnimateRide(u8 *object) {
    u8 *const ride = at<u8 *>(object, 0xb4);
    if (!ride) return;
    g_calls.Cd(0x0044e380, Address(ride), 1u);
    at<u32>(ride, 4) |= 0x200000;
}
}
