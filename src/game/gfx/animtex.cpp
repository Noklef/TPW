#include "animtex.h"

#include <cstdint>

namespace {

u32 *frames() { return reinterpret_cast<u32 *>((uintptr_t)0x008c4b98); }
u32 *counts() { return reinterpret_cast<u32 *>((uintptr_t)0x008c4b80); }

}  // namespace

extern "C" {

// Each animation's frames, then its count.
void __cdecl AnimTextures_Set(const u32 *from, const u32 *fromCounts) {
    for (int animation = 0; animation < 6; ++animation) {
        for (int i = 0; i < 0x100; ++i)
            frames()[animation * 0x100 + i] = from[animation * 0x100 + i];
        counts()[animation] = fromCounts[animation];
    }
}

void __cdecl AnimTextures_Clear() {
    for (int animation = 0; animation < 6; ++animation)
        counts()[animation] = 0;
}
}
