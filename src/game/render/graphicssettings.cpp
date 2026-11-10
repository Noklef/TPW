#include "graphicssettings.h"

#include <cstdint>

#include "../core/balancefile.h"
#include "../core/options.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../files/memsource.h"
#include "../terrain/landscape.h"
#include "viewport.h"
#include "state.h"
#include "../core/staticobjects.h"

namespace {

u32 &global(u32 address) { return *reinterpret_cast<u32 *>(address); }
u32 setting(u32 i) { return global(((u32)(uintptr_t)&gBalanceGraphics + 0x10) + 4 * i); }

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
    const u32 kFlags = (u32)(uintptr_t)&gTexSettings_8bd550, kOther = reinterpret_cast<uintptr_t>(&gGraphicssettings_7a1a8c), kMode = 0x0079d86c, kView = (u32)(uintptr_t)&gView_8bcbc8;
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
    if (v <= 2) global((u32)(uintptr_t)&gTexSettings_8bd550 + 0x40dc) = v == 0 ? 1500 : v == 1 ? 2000 : 2500;
    flag(setting(3), kFlags, 0x40);
    v = setting(4);
    if (v == 0 || v == 1 || v == 2 || v == 4) Landscape_SetDetail(reinterpret_cast<u8 *>((u32)(uintptr_t)&gLandscape_8c1828), nullptr, v);
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
        global((u32)(uintptr_t)&gView_8bcbc8 + 0x8f8) = kFog[v];
        View_SetDepthRange(reinterpret_cast<u8 *>(kView), nullptr, kNear[v], kFar[v]);
    }
    v = setting(16);
    if (v == 0)
        global(kView) |= 0x1800;
    else if (v == 1)
        global(kView) &= ~0x1800u;
}

u32 __cdecl GraphicsSettings_Load(u32 flags) {
    String path;
    String_Ctor(&path);
    vcall<void>(&path, 0x18, MemorySource_GetLock(reinterpret_cast<MemorySource *>((uintptr_t)&gStrings5_f816c0)));   // (String's AssignWide)
    String_EnsureTrailingBackslash(&path);
    if (flags & 0x800)
        vcall<void>(&path, 0x28, (u32)(uintptr_t)&gString_79e3d8);
    else if (flags & 0x1000)
        vcall<void>(&path, 0x28, (u32)(uintptr_t)&gString_79e408);
    else if (flags & 0x2000)
        vcall<void>(&path, 0x28, (u32)(uintptr_t)&gString_79e660);
    if (!BalanceFile_Load(reinterpret_cast<u8 *>((uintptr_t)&gBalanceGraphics), nullptr, path.data, 0)) {
        String_Dtor(&path);
        return 0;
    }
    GraphicsSettings_Apply();
    String_Dtor(&path);
    return 1;
}

u32 __cdecl GraphicsSettings_LoadForOptions() {
    u8 *const options = reinterpret_cast<u8 *>((uintptr_t)&gOptions_78d8d8);
    global(0x0079d86c) &= ~0x400u;
    global((u32)(uintptr_t)&gTexSettings_8bd550 + 0x40d0) = 0;
    gGraphicssettings_7a1a8c |= 0x40604;
    const u32 mode = *reinterpret_cast<u32 *>(options + 8);
    u32 flags = 0;
    if (mode == 0) {
        flags = 0x800;
    } else if (mode == 1) {
        flags = 0x1000;
    } else if (mode == 2) {
        if (*reinterpret_cast<u32 *>(options) == 0) {   // (no procedural terrain: the medium mode instead)
            Options_Set8(options, nullptr, 1);
            flags = 0x1000;
        } else {
            flags = 0x2000;
        }
    }
    if (flags && !GraphicsSettings_Load(flags))
        return 0;
    if (*reinterpret_cast<u32 *>(options + 0xc) == 1)
        gGraphicssettings_7a1a8c |= 0x1000;
    else
        gGraphicssettings_7a1a8c &= ~0x1000u;
    if (*reinterpret_cast<u32 *>(options) == 0) {
        global((u32)(uintptr_t)&gView_8bcbc8) &= 0xfffefffe;
        global(0x008bd524) = 4;
        gGraphicssettings_7a1a8c &= 0xffed1fff;
        global((u32)(uintptr_t)&gTexSettings_8bd550) &= ~2u;
        global(0x0079d86c) &= ~0x200u;
        return 1;
    }
    global(0x008bd524) = 1;
    return 1;
}
}
