#include "helpbg.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/vcall.h"
#include "../ui/frame.h"
#include "../ui/guirect.h"
#include "../ui/window.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall HelpBg_Refresh(u8 *self) {
    if (at<u16>(self, 4) != 0)
        vcall<void>(self, 0);
}

u32 __fastcall Obj_Clear18_0c(u8 *object) {
    at<u32>(object, 0x18) = 0;
    at<u16>(object, 0xc) = 0;
    return 0;
}

void __fastcall HelpBg_SetInterval(u8 *self, void *, u32 value) {
    if (at<u16>(self, 0xc) == 0) return;
    Frame_SetInterval(self, nullptr, value);
    vcall<void>(self, 0);
}

void __fastcall HelpBg_Close(u8 *self) {
    if (at<u16>(self, 0xc) == 0) return;
    if (u8 *const window = at<u8 *>(self, 0x18)) Window_SetShown(window, nullptr, 0);
    Fn_669f94(self);
}

void __fastcall HelpBg_Show(u8 *self) {
    if (at<u16>(self, 0xc) == 0) return;
    Fn_66a002(self);
    Window_SetShown(at<u8 *>(self, 0x18), nullptr, 1);
}

void __fastcall HelpBg_Hide(u8 *self) {
    if (at<u16>(self, 0xc) == 0) return;
    Frame_Stop(self);
    Window_SetShown(at<u8 *>(self, 0x18), nullptr, 0);
}

u32 __fastcall HelpBg_MoveTo(u8 *self, void *, u32 x, u32 y) {
    if (!at<u8 *>(self, 0x18)) return 0;
    u8 rect[12];
    GuiRect_Ctor(rect);
    Window_GetRect(at<u8 *>(self, 0x18), nullptr, rect, 1);
    const u32 left = at<u32>(rect, 4), top = at<u32>(rect, 6);  // (dwords: left with top, top with right)
    if ((u16)x == 0xffff) x = left;
    if ((u16)y == 0xffff) y = top;
    Window_Offset(at<u8 *>(self, 0x18), nullptr, x - left, y - top);
    return GuiRect_Dtor(rect);
}

s32 __cdecl HelpBg_WindowHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) vcall<u32>(reinterpret_cast<u8 *>(Window_DwordA0(window, nullptr, 0)), 0x3c);
    return Window_CallDefault(window, nullptr, message, a, b);
}

void __cdecl HelpBg_SetText(const u16 *text) {
    u8 *const bg = reinterpret_cast<u8 *>(0x007ca0c0);
    if (!at<u16>(bg, 0xc) || !at<u16>(bg, 4)) return;
    const u16 *p = text;
    while (*p && *p != '\n') ++p;  // wcschr(text, '\n') (0x67c2f0), inlined
    Window_NotifyC00(at<u8 *>(bg, 0x18), nullptr, 1, *p ? 0 : 1);
    if (!Crt().wcslen(text)) {
        vcall<void>(bg, 4);
        return;
    }
    Window_SetShown(at<u8 *>(bg, 0x18), nullptr, 1);
    vcall<void>(at<u8 *>(bg, 0x18), 0x24, text);
}
}  // extern "C"
