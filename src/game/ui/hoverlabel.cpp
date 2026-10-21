#include "hoverlabel.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/initializers.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"

namespace {

u8 *sharedPart() { return reinterpret_cast<u8 *>((uintptr_t)0x007c2118); }

}  // namespace

extern "C" {

void __cdecl HoverLabel_SetYellow(u8 *window) { Window_Notify40000(window, nullptr, 0xea, 0xef, 0x66, 0xff); }

void __cdecl HoverLabel_Attach(u8 *parent, u32 id, u32 text) {
    u8 *const child = Window_FindChild(parent, nullptr, id);
    if (!child) return;
    if (!*reinterpret_cast<u8 **>(child + 0x98)) {
        u8 rect[0x10];
        GuiRect_Ctor(rect);
        Window_GetRect(child, nullptr, rect, 1);
        const s16 left = *reinterpret_cast<s16 *>(rect + 4), top = *reinterpret_cast<s16 *>(rect + 6);
        const s16 right = *reinterpret_cast<s16 *>(rect + 8), bottom = *reinterpret_cast<s16 *>(rect + 0xa);
        const s32 half = ((s32)right - (s32)left) / 2;
        Fn_659dac(rect, nullptr, (u16)(left - half), (u16)top, (u16)(right + half), (u16)bottom);
        Window_SetRect(child, nullptr, rect);
        Set4c_48f690(sharedPart(), nullptr, 0);
        Window_SetObject98(child, nullptr, sharedPart());
        Window_Set4c(child, nullptr, Language_Font(5));
        Window_NotifyC00(child, nullptr, 1, 1);
        HoverLabel_SetYellow(child);
        GuiRect_Dtor(rect);
    }
    Window_NotifyObject98_100(child, nullptr, text);
}

s32 __cdecl HoverLabel_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x10001) {
        Window_Notify40000(window, nullptr, 0xff, 0xff, 0xff, 0xff);
    } else if (message == 0x10002) {
        HoverLabel_SetYellow(window);
    } else if (message == 0x10006 && a == 0) {
        vcall<void>(reinterpret_cast<u8 *>((uintptr_t)Window_DwordA0(window, nullptr, 0)), 0x38);
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}

void __fastcall HoverLabel_Bind(u8 *owner, void *, u32 id) {
    u8 *const child = Window_FindChild(*reinterpret_cast<u8 **>(owner + 0x10), nullptr, id);
    *reinterpret_cast<u8 **>(owner + 0x20) = child;
    Window_SetHandler(child, nullptr, 0x0048d2e0);
    Window_SetDwordA0(*reinterpret_cast<u8 **>(owner + 0x20), nullptr, 0, (u32)(uintptr_t)owner);
    HoverLabel_Attach(*reinterpret_cast<u8 **>(owner + 0x10), id, 0x0078a374);
}
}  // extern "C"
