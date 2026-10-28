#include "dialog7ca13c.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/filetime.h"
#include "../core/funnytime.h"
#include "../core/language.h"
#include "../core/vwrap.h"
#include "../world/advisor.h"
#include "../world/person.h"
#include "../world/ridesweep.h"
#include "buttontext.h"
#include "focus.h"
#include "listrows.h"
#include "thingbrowser.h"
#include "window.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "windowscript.h"
#include "windowtimers.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename T> T field(const u8 *object, u32 offset) { return *reinterpret_cast<const T *>(object + offset); }

}  // namespace

extern "C" {

void __cdecl Dialog7ca13c_Open(u32 selected) {
    Window7c24c8_Close();
    Global7cb28c_Select(2, 6);
    u8 *const dialog = WindowScript_Build(global<u8 *>(0x007cb2ac), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x007506c8), 0x00493230);
    if (!dialog) return;
    global<u8 *>(0x007ca13c) = dialog;
    Window7c24c8_Set(dialog);
    if (u8 *const list = Window_FindChild(global<u8 *>(0x007ca13c), nullptr, 0x1e497)) {
        Window7059f0_BuildRows(list, nullptr, &List7ca140_NewRow, &List7ca140_InitRow);   // (0x4938c0, 0x4939d0: only called, not stored)
        for (u32 i = 0; i < 6; ++i) Window7059f0_SetColumnBottom(list, nullptr, i, 1);
        Window7059f0_SetHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>(0x0074fa84))));
        Window7059f0_SortBy(list, nullptr, global<s32>(0x007508bc), 0);
        u8 *const again = Window_FindChild(global<u8 *>(0x007ca13c), nullptr, 0x1e497);
        global<u8 *>(0x007ca138) = again;
        global<u32>(0x007ca134) = 0;
        Things_ForEachSelected(0x40, reinterpret_cast<ObjectCallback>((uintptr_t)0x004937e0));
        Window7059f0_RefreshRows(again);
        if (selected) {
            const s32 position = Window7059f0_FindItemData(list, nullptr, selected);
            if (position >= 0) Window7059f0_ScrollTo(list, nullptr, position);
        }
        for (u32 i = 0; i < 6; ++i) {   // (the header buttons: the list's children 0x10-0x15)
            u8 *const button = Window_FindChild(list, nullptr, 0x10 + i);
            const u32 font = global<u32>(0x007508b8);
            u16 *const text = Language_WideText(0x71 + i);
            ButtonText_Init(button, (u32)(uintptr_t)text, font, 0xff, 0xef, 0, 0x81 + i);
        }
    }
    if ((u16)Window_SetTimer(global<u8 *>(0x007ca13c), nullptr, 0x80083, 2000)) Window_StartTimer(global<u8 *>(0x007ca13c), nullptr, 0x80083);
    Advisor_PostId(0x11e);
}

void __cdecl Dialog7ca13c_RemoveData(u32 data) {
    if (!global<u8 *>(0x007ca13c)) return;
    u8 *const list = Window_FindChild(global<u8 *>(0x007ca13c), nullptr, 0x1e497);
    const s32 position = Window7059f0_FindItemData(list, nullptr, data);
    if (position < 0) return;
    Window7059f0_RemoveItem(list, nullptr, position);
    --global<s32>(0x007ca134);
}

void __cdecl Dialog7ca13c_AddRow(u8 *guest) {
    FileTime span;
    const FileTime *const s = FunnyTime_SpanOf(global<u8 *>(0x007cf83c) + 0x2a0, nullptr, &span, Visitor_TimeInPark(guest));
    const long long hours = (long long)(((unsigned long long)s->high << 32) | s->low) / 36000000000LL;   // (_alldiv: an hour of ticks)
    const u32 left = field<u32>(guest, 0x30);
    const u32 happy = (u32)(long long)field<float>(guest, 0x19c) & 0xff;   // (FLD, __ftol's low byte)
    const u32 texts[6] = {field<u32>(guest, 0x1d8), field<u32>(guest, 0x1a0), (u32)hours, field<u32>(guest, 0x1c4), left ? left - 1 : 999,
                          (u32)((s32)(happy << 10) / 100)};
    Window7059f0_AddItem(global<u8 *>(0x007ca138), nullptr, texts, (u32)(uintptr_t)guest, -1, 0);
}

void __cdecl Dialog7ca13c_AddGuest(u8 *guest) {
    if (!global<u8 *>(0x007ca13c)) return;
    global<u8 *>(0x007ca138) = Window_FindChild(global<u8 *>(0x007ca13c), nullptr, 0x1e497);
    Dialog7ca13c_AddRow(guest);
    Window7059f0_RefreshRows(global<u8 *>(0x007ca138));
}

void __fastcall Obj7b51f0_OnGuestAdded(void *, void *, u32 handle) {
    Dialog7ca13c_AddGuest(global<u8 *>(0x007cfb90 + (handle & 0xffff) * 20));   // (the original's empty assert that the table is there left out)
}

void __fastcall Obj7b51f0_OnGuestRemoved(void *, void *, u32 handle) {
    u8 *const thing = global<u8 *>(0x007cfb90 + (handle & 0xffff) * 20);   // (the original's empty assert that the table is there left out)
    if (thing[2] != 1) return;
    Obj_CallSlot2cIfGlobalIf10(reinterpret_cast<u8 *>((uintptr_t)0x007cc408), nullptr, (u32)(uintptr_t)thing);
    Dialog7ca13c_RemoveData((u32)(uintptr_t)thing);
    ThingBrowser_ResetIfThing(thing);
}
}  // extern "C"
