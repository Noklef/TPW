#include "scream.h"

#include <cstdint>

#include "../core/soundplay.h"
#include "../core/soundset.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 volume(s32 a, s32 b) {
    const s32 mean = static_cast<s32>(static_cast<u32>(a) + static_cast<u32>(b)) / 2;
    return mean > 100 ? 100 : mean < 0 ? 0 : mean;
}

}  // namespace

extern "C" {

u32 __cdecl RSSE_StartScream(u32 scream, s32 level, s32 a, s32 b, s32 x, s32 y, s32 z) {
    if (scream)   // (the original gives "RSSE: Started screaming without stopping first." to the empty Debug_Printf)
        return 0;
    if (level == 0)
        return 0;
    const u32 sound = level == 1 ? 0x47 : level < 4 ? 0x48 : level < 8 ? 0x49 : 0x4a;
    const u32 played = Sound_PlayAt(0, global<u32>(0x00803a24), sound, x, y, z);
    if (played)
        Sound_SetParam(played, 6, static_cast<u32>(volume(a, b)));
    return played;
}

u32 __cdecl RSSE_SetScreamVolume(u32 scream, s32 a, s32 b) {
    return scream ? Sound_SetParam(scream, 6, static_cast<u32>(volume(a, b))) : 0;
}

u32 __cdecl RSSE_MoveScream(u32 scream, s32 x, s32 y, s32 z) { return scream ? Sound_SetPosition(scream, x, y, z) : 0; }
}
