// The park's random number generator: reimplementation of 0x516330 and 0x516370.
#include "random.h"

namespace {

u32 &state(u8 *world) { return *reinterpret_cast<u32 *>(world + 0x1da708); }

}  // namespace

extern "C" s32 __fastcall World_Rand(u8 *world) {
    const u32 next = state(world) * 0x19660du + 0x3c6ef35fu;
    state(world) = next;
    const u32 turned = next >> 13 | next << 19;
    state(world) = turned;
    const s32 v = (s32)turned;
    return v < 0 ? (s32)(0u - turned) : v;
}

extern "C" void __fastcall World_SetSeed(u8 *world, void *, u32 seed) { state(world) = seed; }
