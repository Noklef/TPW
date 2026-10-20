#include "panel7cc328.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../sprite/script.h"
#include "../world/ridesweep.h"
#include "../world/staff.h"
#include "../world/things.h"
#include "window.h"
#include "window7059f0.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall Obj_KillSlot24(u8 *object) {
    if (!at<u32>(object, 0x24))
        return 0;
    SpriteScript_Kill((s32)at<u32>(object, 0x24));
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x28) = 0xffffffff;
    at<u32>(object, 0x2c) = 0xffffffff;
    return 0xffffffff;
}

u32 __cdecl Panel7cc328_AddStaffRow(u8 *staff) {
    if (!Staff_IsState3At(staff, nullptr, global<u16>(0x007cc310))) return 1;
    u32 texts[2];
    texts[0] = (u32)(uintptr_t)Staff_GetName(staff);
    const u32 low = (u32)(long long)at<float>(staff, 0x1fc) & 0xff;   // (__ftol's low byte)
    texts[1] = (u32)((s32)(low << 10) / 100);
    Window7059f0_AddItem(global<u8 *>(0x007cc314), nullptr, texts, (u32)(uintptr_t)staff, -1, 0);
    return 1;
}

s32 __cdecl Panel7cc328_OnMessage(u8 *window, u32 message, u32 id, u32 b) {
    if (message == 0x101 && b == 1 && id - 0x1e7d <= 4) {
        static const u32 kTab[5] = {0, 4, 2, 1, 3}, kMask[5] = {2, 0x20, 0x10, 4, 8};   // (by id - 0x1e7d: the tab and its staff group)
        const u32 k = id - 0x1e7d;
        u8 *const list = Window_FindChild(global<u8 *>(0x007cc338), nullptr, 0x1e7b);
        Window7059f0_Clear(list);
        global<u8 *>(0x007cc314) = list;
        u16 handle;
        global<u16>(0x007cc310) = *Thing_GetHandle(global<u8 *>(0x007c2658), nullptr, &handle);
        global<u32>(0x007cc358) = kTab[k];
        Things_ForEachSelected(kMask[k], reinterpret_cast<ObjectCallback>(&Panel7cc328_AddStaffRow));
        Window7059f0_RefreshRows(list);
        vcall<void>(reinterpret_cast<u8 *>((uintptr_t)0x007cc328), 0x20);
    }
    return Window_CallDefault(window, nullptr, message, id, b);
}
}  // extern "C"
