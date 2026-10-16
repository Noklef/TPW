#include "state.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall SceneFlags_Touch(u32 *flags) {
    flags[1] = global<u32>(0x008bcc40);
    flags[0] |= 0x40000000;
}

void __fastcall Render_FogOn(void *) { global<u32>(0x008bcbc8) |= 0x2000; }

void __fastcall Render_FogOff(void *) { global<u32>(0x008bcbc8) &= ~0x2000u; }

// The start and end are floats, moved as dwords.
void __fastcall Render_SetFog(void *, void *, u32 colour, u32 start, u32 end) {
    global<u32>(0x008bd47c) = colour;
    global<u32>(0x008bd490) = start;
    global<u32>(0x008bd494) = end;
}
}
