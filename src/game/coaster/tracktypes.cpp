#include "tracktypes.h"

#include <cstdint>

#include "../core/vwrap.h"
#include "../render/particles.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

s32 __cdecl TrackTypeSlot_ReadIntoFree(u8 *stream) {
    if (gParticles_816d40 == 0)
        return -1;
    s32 index = 0;
    for (u32 slot = ((u32)(uintptr_t)&gParticleEffects + 0x118); slot != 0 && global<u8>(slot) != 0;) {
        slot += 0x140;
        index++;
        if ((s32)slot > (s32)((u32)(uintptr_t)&gParticleEffects + 0x8457))
            return -1;
    }
    Object_CallSlot8Product((u32)(uintptr_t)&gParticleEffects + (u32)index * 0x140, 0x140, 1, stream);
    return index;
}

}  // extern "C"
