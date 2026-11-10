#include "rideskin.h"

#include <cstdint>

#include "rides.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const RideSkinEnv kPorts = {&Ride_Destroy};
const RideSkinEnv *g_env = &kPorts;

void Replace(const u8 *model, u32 from, u32 to) {
    const u32 parts = at<u16>(model, 0x44);
    const u8 *part = at<const u8 *>(model, 0x70) + 0x5a;
    for (u32 i = 0; i < parts; ++i, part += 0xa0) {
        u8 *entry = at<u8 *>(part, 0x12);
        for (u32 n = at<u16>(part, 0); n != 0; --n, entry += 0x10) {
            if (at<u32>(entry, 0) == from) at<u32>(entry, 0) = to;
        }
    }
}

}  // namespace

void RideSkin_SetEnv(const RideSkinEnv *env) { g_env = env ? env : &kPorts; }

extern "C" void __cdecl Ride_RestoreSkinsAndDestroy(u8 *record) {
    if (!record) return;
    const u32 kind = at<u32>(record, 0x84);
    if (kind != 0 && at<u32>(at<const u8 *>(reinterpret_cast<const void *>((uintptr_t)0x007ae6a8), kind * 4), 0xd0) != 0) {
        const u8 *const instance = gRides_7a4610[at<u32>(record, 4)];
        const u8 *const model = at<const u8 *>(at<const u8 *>(instance, 8), 4);
        Replace(model, at<u32>(record, 0x7c), at<u32>(record, 0x74));
        Replace(model, at<u32>(record, 0x80), at<u32>(record, 0x78));
        at<u32>(record, 0x84) = 0;
    }
    g_env->destroy(at<u32>(record, 4));
}
