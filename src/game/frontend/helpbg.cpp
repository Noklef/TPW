#include "helpbg.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../ui/frame.h"
#include "../ui/guirect.h"
#include "../render/textparts.h"
#include "../ui/window.h"
#include "../ui/windowfactory.h"
#include "../ui/windowreg.h"

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

u32 __fastcall HelpBg_RestoreRect(u8 *self) {
    if (!at<u8 *>(self, 0x18)) return 0;
    u8 rect[12];
    GuiRect_Ctor(rect);
    Window_GetRect14(at<u8 *>(self, 0x18), nullptr, rect);
    Window_SetRect(at<u8 *>(self, 0x18), nullptr, rect);
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

void __fastcall HelpBg_Build(u8 *self) {
    const s32 height = (s16)vcall<u32>(Language_Font(7), 4) * 0xc00 / *reinterpret_cast<const s16 *>((uintptr_t)0x00faa5c0) + 10;
    at<u8 *>(self, 0x18) = Window_CreateChild(*reinterpret_cast<u8 **>((uintptr_t)0x00faa5d8), nullptr, 1, 1, 0xfff355e4, 0x23e,
                                              (u32)(0x5f0 - height), 0x5c2, 0x5f0);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(at<u8 *>(self, 0x18), nullptr, memory ? TextPart_Ctor(memory, nullptr, 0) : nullptr);
    Window_NotifyC00(at<u8 *>(self, 0x18), nullptr, 1, 1);
    Window_Notify3000(at<u8 *>(self, 0x18), nullptr, 1, 1);
    Window_Set4c(at<u8 *>(self, 0x18), nullptr, Language_Font(7));
    Window_Notify40000(at<u8 *>(self, 0x18), nullptr, 0xff, 0xff, 0xff, 0xff);
    Window_NotifyImpl100(at<u8 *>(self, 0x18), nullptr, 0);
    Window_SetObject98(at<u8 *>(self, 0x18), nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName("f_helpbg")));
    Window_SetDwordA0(at<u8 *>(self, 0x18), nullptr, 0, (u32)(uintptr_t)self);
    Window_SetDepth(at<u8 *>(self, 0x18), nullptr, 0x26);
    vcall<void>(at<u8 *>(self, 0x18), 0x14, 0u);
    Window_SetHandler(at<u8 *>(self, 0x18), nullptr, 0x00491fe0);
    at<u16>(self, 0xc) = 1;
}
}  // extern "C"
