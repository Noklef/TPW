#include "state.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

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

u32 __fastcall Render_FadeFog(u8 *fade) {
    u32 colour = global<u32>(0x008bd47c);
    if (global<s32>(0x00785888) > 1 && global<u32>(0x0078d8d8) == 1) {
        const s32 a = at<s32>(fade, 0x44), b = at<s32>(fade, 0x48), c = at<s32>(fade, 0x4c);
        const s32 targetB = at<s32>(fade, 0x54), targetC = at<s32>(fade, 0x58);
        if (a != at<s32>(fade, 0x50) || b != targetB || c != targetC) {
            const s32 na = a + ((targetC - a) >> 4);  // sic: +0x58, not +0x50
            const s32 nb = b + ((targetB - b) >> 4);
            const s32 nc = c + ((targetC - c) >> 4);
            at<s32>(fade, 0x44) = na;
            at<s32>(fade, 0x48) = nb;
            at<s32>(fade, 0x4c) = nc;
            colour = ((((0xffffff00u | (u32)(na >> 16)) << 8) | (u32)(nb >> 16)) << 8) | (u32)(nc >> 16);
        }
    }
    for (u32 i = 0; i < 0x100; i++)
        global<u32>(0x008c2788 + 4 * i) = colour;
    return colour;
}
}
