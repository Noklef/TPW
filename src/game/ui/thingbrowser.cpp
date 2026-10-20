#include "thingbrowser.h"

#include <cstdint>

#include "../core/idmaps.h"
#include "../core/textarg.h"
#include "../core/vcall.h"
#include "../world/camera.h"
#include "../world/staff.h"
#include "window.h"

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

bool is_staff(u8 kind) { return kind == 4 || kind == 5 || kind == 6 || kind == 8 || kind == 7; }

u16 *const kText = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);
}  // namespace

extern "C" {

void __cdecl ThingBrowser_ShowName() {
    if (!global<u8 *>(0x007cc308)) return;
    u8 *const label = Window_FindChild(global<u8 *>(0x007cc304), nullptr, 0x147);
    u8 *const thing = global<u8 *>(0x007cc308);
    if (is_staff(thing[2])) {
        vcall<void>(label, 0x24, Staff_GetName(thing));
    } else {
        const u32 count = at<u32>(thing, 0x1d8);
        if (!count) {
            TextFormat().format(global<void *>(0x0078ba94), kText, 0x3fff, 0x1c9);
        } else {
            u8 arg[0x14];  // (a number argument, as TextArgInt_Ctor (n 0x18, count, unsigned) builds it inline)
            at<u32>(arg, 0) = 0x006ff170;
            at<u32>(arg, 4) = 0x5641524d;
            at<u32>(arg, 8) = 0x18;
            at<u32>(arg, 0xc) = count;
            at<u32>(arg, 0x10) = 0;
            TextFormat().format(global<void *>(0x0078ba94), kText, 0x3fff, 0x1c8, arg);
        }
        vcall<void>(label, 0x24, kText);
    }
    ThingBrowser_ShowDetail();
    Camera_CentreOnSprite(global<u8 *>(0x007cc308));
}

void __cdecl ThingBrowser_ShowDetail() {
    u8 *const thing = global<u8 *>(0x007cc308);
    if (!thing) return;
    if (is_staff(thing[2])) {
        const u32 code = Index_ToCode485ec0(Mode19c_ToIndex(thing));
        if (code == global<u32>(0x00756b98)) return;
        global<u32>(0x00756b98) = code;
        Window_Notify1(Window_FindChild(global<u8 *>(0x007cc304), nullptr, 0x148), nullptr, code);
        return;
    }
    const u32 count = at<u32>(thing, 0x30);
    u8 *const child = Window_FindChild(global<u8 *>(0x007cc304), nullptr, 0x14a);
    vcall<void>(child, 0x1c, count ? count - 1 : 0x29au);
}
}
