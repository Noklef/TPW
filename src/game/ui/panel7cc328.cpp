#include "panel7cc328.h"

#include <cstdint>
#include <cstring>

#include "../core/idmaps.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "../sprite/screenmatrix.h"
#include "../sprite/script.h"
#include "../world/advisor.h"
#include "../world/object.h"
#include "../world/ridesweep.h"
#include "../world/staff.h"
#include "../world/things.h"
#include "dialogs.h"
#include "guirect.h"
#include "panelparts.h"
#include "window.h"
#include "window7059f0.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall Panel7cc328_ShowRefund(u8 *self) {
    u8 *const thing = global<u8 *>(0x007c2658);
    u8 *const child = Window_FindChild(at<u8 *>(self, 0x14), nullptr, 0x1e8a);
    vcall<void>(child, 0x1c, Object_ScrapRefund(thing));
}

u32 __fastcall Obj_KillSlot24(u8 *object) {
    if (!at<u32>(object, 0x24))
        return 0;
    SpriteScript_Kill((s32)at<u32>(object, 0x24));
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x28) = 0xffffffff;
    at<u32>(object, 0x2c) = 0xffffffff;
    return 0xffffffff;
}

void __fastcall Panel7cc328_ShowStaff(u8 *panel, void *, const u8 *staff) {
    u8 rect[0xc];
    GuiRect_Ctor(rect);
    u8 *const child = Window_FindChild(at<u8 *>(panel, 0x10), nullptr, 0x1e84);
    Window_GetSavedRect(child, nullptr, rect);
    const s32 left = at<s16>(rect, 4), top = at<s16>(rect, 6), right = at<s16>(rect, 8), bottom = at<s16>(rect, 0xa);
    const float x = (float)X87_MulIntDouble(((right - left) >> 1) + left - 0x3f8, global<float>(0x006ffdcc));
    const float y = (float)X87_MulIntDouble(((bottom - top) >> 2) - bottom + 0x300, global<float>(0x006ffdd0));
    const float z = (float)Sprite_FlipY(at<s16>(child, 0xe0) + 1);
    const u32 folder = at<u32>(staff, 0x24), sprite = at<u32>(staff, 0x20);
    if (folder != at<u32>(panel, 0x28) || sprite != at<u32>(panel, 0x2c)) {
        at<u32>(panel, 0x28) = folder;
        at<u32>(panel, 0x2c) = sprite;
        if (at<u32>(panel, 0x24)) SpriteScript_Kill(at<s32>(panel, 0x24));
        u32 bits[3];
        std::memcpy(&bits[0], &x, 4);
        std::memcpy(&bits[1], &y, 4);
        std::memcpy(&bits[2], &z, 4);
        at<s32>(panel, 0x24) = SpriteScript_Start(0x0074f758, folder, sprite, 1, bits[0], bits[1], bits[2]);
        SpriteScript_SetRegF(at<s32>(panel, 0x24), 8, 0x41000000);
    }
    GuiRect_Dtor(rect);
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

void __fastcall Panel7cc328_Reopen(u8 *self) {
    void *const *const vtable = at<void *const *>(at<u8 *>(self, 0x20), 0);   // (read before slot 13, the object after: as the original)
    const u32 value = vcall<u32>(self, 0x34);
    reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>(vtable[9])(at<u8 *>(self, 0x20), nullptr, value);
    vcall<void>(self, 0x18);
    const u32 tab = global<u32>(0x007cc358);
    u8 *const list = Window_FindChild(at<u8 *>(self, 0x10), nullptr, 0x1e7b);
    Window7059f0_Clear(list);
    global<u8 *>(0x007cc314) = list;
    u16 handle;
    global<u16>(0x007cc310) = *Thing_GetHandle(global<u8 *>(0x007c2658), nullptr, &handle);
    global<u32>(0x007cc358) = tab;
    static const u32 kMask[5] = {2, 4, 0x10, 8, 0x20};   // (by the tab)
    if (tab <= 4) Things_ForEachSelected(kMask[tab], reinterpret_cast<ObjectCallback>(&Panel7cc328_AddStaffRow));
    Window7059f0_RefreshRows(list);
    vcall<void>(self, 0x20);
    vcall<void>(self, 0x14);
}

void __fastcall Panel7cc328_AddStaff(u8 *self, void *, u8 *staff) {
    if (!at<u8 *>(self, 0x10)) return;
    u8 *const list = Window_FindChild(at<u8 *>(self, 0x10), nullptr, 0x1e7b);
    if (!Staff_IsState3At(staff, nullptr, global<u16>(0x007cc310))) return;
    if (Byte2_ToIndex(staff) != global<u32>(0x007cc358)) return;
    u32 texts[2];
    texts[0] = (u32)(uintptr_t)Staff_GetName(staff);
    const u32 low = (u32)(long long)at<float>(staff, 0x1fc) & 0xff;   // (__ftol's low byte)
    texts[1] = (u32)((s32)(low << 10) / 100);
    Window7059f0_AddItem(list, nullptr, texts, (u32)(uintptr_t)staff, -1, 1);
}

void __fastcall Panel7cc328_RemoveStaff(u8 *self, void *, u8 *staff) {
    if (!at<u8 *>(self, 0x10)) return;
    u8 *const list = Window_FindChild(at<u8 *>(self, 0x10), nullptr, 0x1e7b);
    const s32 n = Window7059f0_FindItemData(list, nullptr, (u32)(uintptr_t)staff);
    if (n >= 0) Window7059f0_RemoveItem(list, nullptr, n);
}

void __fastcall Panel7cc328_OnStaff(void *, void *, u32 handle, u32 added) {
    u8 *const staff = global<u8 *>(0x007cfb90 + (handle & 0xffff) * 20u);
    u8 *const panel = reinterpret_cast<u8 *>((uintptr_t)0x007cc328);
    if (added)
        Panel7cc328_AddStaff(panel, nullptr, staff);
    else
        Panel7cc328_RemoveStaff(panel, nullptr, staff);
}

void __fastcall Panel7cc328_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007cc318) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00756ba0), 0x004b4fc0, 0x1e84, 0x1e89, 0);
    Advisor_PostId(0x12c);
}

void __fastcall Panel7cc328_Dress(u8 *self) {
    panelparts::text(at<u8 *>(self, 0x14), 0x1e8b, 0x38);
    panelparts::part(at<u8 *>(self, 0x14), TextPart6ff3b0_Ctor, 0x1e8a, 0);
}
}  // extern "C"
