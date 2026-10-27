#include "rides.h"

#include <cstdint>

#include "../model/highlight.h"
#include "../model/holder.h"
#include "camera.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 **instances() { return reinterpret_cast<u8 **>(0x007a4610); }

const RidesEnv kGameEnv = {reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x0042a190), reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)0x00436fa0)};
const RidesEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void Rides_SetEnv(const RidesEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Ride_Destroy(u32 ride) {
    if (ride == 0) return;
    u8 *const instance = instances()[ride];
    if (!instance) return;
    if (Highlight_Current() == instance) Highlight_Clear();
    if (instance == global<u8 *>(0x007afda0)) global<u8 *>(0x007afda0) = nullptr;
    if (Camera_IsFollowing(reinterpret_cast<const void *>((uintptr_t)ride))) g_env->stopFollowing();
    if (at<u32>(instance, 4) & 0x4000000u) Highlight_Unflash(instance);
    g_env->forgetCoaster(ride);
    Instance_Dispose(instance);
    instances()[ride] = nullptr;
    const u32 slot = ride - 1;
    u32 holes = global<u32>(0x007a3ebc);
    u32 count = global<u32>(0x007a4084);
    if (holes == 0) {
        --count;
        if (slot == count) {
            global<u32>(0x007a4084) = count;
            --global<u32>(0x007aedf4);
        } else {
            global<u32>(0x007a3ebc) = 1;
            global<u32>(0x007aedf4) = slot;
            global<u32>(0x007a4084) = count;
        }
        return;
    }
    if (slot == holes + count - 1) {
        global<u32>(0x007a4084) = --count;
        for (u8 **entry = &instances()[count + holes]; *entry == nullptr; --entry) {
            if (--holes == 0) break;
        }
        global<u32>(0x007a3ebc) = holes;
        return;
    }
    global<u32>(0x007a3ebc) = holes + 1;
    global<u32>(0x007a4084) = count - 1;
    if (slot < global<u32>(0x007aedf4)) global<u32>(0x007aedf4) = slot;
}
}
