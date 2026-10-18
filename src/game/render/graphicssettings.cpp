#include "graphicssettings.h"

#include "../terrain/landscape.h"
#include "viewport.h"

namespace {

u32 &global(u32 address) { return *reinterpret_cast<u32 *>(address); }
u32 setting(u32 i) { return global(0x00785878 + 4 * i); }

// A setting of 0 clears the bits of target, 1 sets them, any other value leaves them.
void flag(u32 value, u32 target, u32 bits) {
    if (value == 0)
        global(target) &= ~bits;
    else if (value == 1)
        global(target) |= bits;
}

}  // namespace

extern "C" {

void __cdecl GraphicsSettings_Apply() {
    const u32 kFlags = 0x008bd550, kOther = 0x007a1a8c, kMode = 0x0079d86c, kView = 0x008bcbc8;
    u32 v = setting(0);
    if (v == 0 || v == 1)
        global(kFlags) &= ~0x18u;
    else if (v == 2 || v == 3)
        global(kFlags) |= 0x18;
    v = setting(1);
    if (v == 0) {
        global(kOther) &= ~0x6000u;
    } else if (v >= 1 && v <= 3) {
        global(kOther) |= 0x6000;
        global(kMode) |= v == 1 ? 4 : v == 2 ? 2 : 1;
    }
    v = setting(2);
    if (v <= 2) global(0x008c162c) = v == 0 ? 1500 : v == 1 ? 2000 : 2500;
    flag(setting(3), kFlags, 0x40);
    v = setting(4);
    if (v == 0 || v == 1 || v == 2 || v == 4) Landscape_SetDetail(reinterpret_cast<u8 *>(0x008c1828), nullptr, v);
    flag(setting(5), kView, 0x400000);
    flag(setting(6), kView, 0x10000);
    flag(setting(7), kMode, 0x200);
    v = setting(8);
    if (v == 0)
        global(kView) = (global(kView) & 0xffebffff) | 0x20000;
    else if (v == 1)
        global(kView) = (global(kView) & 0xffedffff) | 0x40000;
    else if (v == 2 || v == 3)
        global(kView) &= 0xffe9ffff;
    flag(setting(9), kView, 1);
    v = setting(10);
    if (v == 0 || v == 1) global(kView) &= ~0x6000u;
    flag(setting(11), kFlags, 2);
    flag(setting(12), kOther, 0x200000);
    flag(setting(13), kOther, 0x400000);
    flag(setting(14), kFlags, 0x20);
    v = setting(15);
    if (v <= 3) {
        static const u32 kFog[] = {0x43480000, 0x43a28000, 0x43e10000, 0x442f0000};               // 200, 325, 450, 700
        static const float kNear[] = {50.0f, 100.0f, 135.0f, 230.0f}, kFar[] = {60.0f, 110.0f, 150.0f, 250.0f};
        global(0x008bd4c0) = kFog[v];
        View_SetDepthRange(reinterpret_cast<u8 *>(kView), nullptr, kNear[v], kFar[v]);
    }
    v = setting(16);
    if (v == 0)
        global(kView) |= 0x1800;
    else if (v == 1)
        global(kView) &= ~0x1800u;
}
}
