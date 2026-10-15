#include "frame.h"

#include "../motion/animator.h"

namespace {

u32 &global(u32 address) { return *reinterpret_cast<u32 *>(address); }

}  // namespace

extern "C" {

void __cdecl Frame_Begin() {
    u8 *const animator = *reinterpret_cast<u8 **>(0x0079fcb4);
    global(0x0079e884) = 0;
    if (animator) Animator_Update(animator, reinterpret_cast<const u8 *>(1), 4);
}

void __cdecl Frame_ResetBounds() {
    for (u32 i = 0; i < 3; ++i) global(0x007b0600 + 4 * i) = 0x7f7fffff;
    for (u32 i = 0; i < 3; ++i) global(0x007b060c + 4 * i) = 0xff7fffff;
}

void __cdecl Frame_AddFlags(u32 flags) { global(0x0079e884) |= flags; }

void __cdecl Frame_SetPoint(const u32 *point) {
    if (point) {
        global(0x0079e248) = point[0];
        global(0x0079e24c) = point[1];
        global(0x0079e250) = point[2];
    }
    global(0x0079dcd4) = 0;
}
}
