#include "state.h"

#include <cstdint>

#include "../core/tiny_a.h"

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

u32 __cdecl Environment_Apply() {
    global<u32>(0x008bd47c) = global<u32>(0x007856d4);
    global<u32>(0x007a0b44) = global<u32>(0x007856d8);
    global<u32>(0x007a0b3c) = global<u32>(0x007856dc);
    const s32 c = global<s32>(0x007856e0), e = global<s32>(0x007856e4);
    global<s32>(0x007a006c) = c > 0 ? c : 0;
    global<s32>(0x007a01c0) = e > 0 ? e : 0;
    global<u32>(0x007a0b30) = global<u32>(0x007856e8);
    global<u32>(0x007a0b34) = global<u32>(0x007856ec);
    global<u32>(0x007a0b38) = global<u32>(0x007856f0);
    global<u32>(0x00870ec8) = global<u32>(0x00785718);
    global<u32>(0x00870ecc) = global<u32>(0x0078571c);
    global<u32>(0x00870ed0) = global<u32>(0x00785720);
    for (u32 i = 0; i < 4; i++) {
        global<u32>(0x00f797a4 + 4 * i) = global<u32>(0x007856f4 + 8 * i);
        global<u32>(0x00f797b4 + 4 * i) = global<u32>(0x007856f8 + 8 * i);
    }
    Global81b6dc_Set_52fb10(global<u32>(0x00785600));
    Global81ea14_Set_52fb30(global<u32>(0x007855fc));
    Global81aebc_Set_52fb50(global<u32>(0x00785604));
    Global81b43c_Set_52fb70(global<u32>(0x00785608));
    return 1;
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
