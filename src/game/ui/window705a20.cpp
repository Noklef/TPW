#include "window705a20.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"
#include "windowtimers.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Window705a20_SaveRect(u8 *window) { GuiRect_Assign(window + 0x2c, nullptr, window + 8); }

u8 *__fastcall Window705a20_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    at<u32>(window, 0) = 0x705a20;
    at<u32>(window, 0x13c) = 0;
    if (flags & 0x40) {
        at<u8 *>(window, 0x138) = static_cast<u8 *>(GameHeap_Alloc(0x200));
        if (at<u8 *>(window, 0x138)) {
            at<u16>(at<u8 *>(window, 0x138), 0) = 0;
            at<u32>(window, 0x13c) = 0xff;
        }
    } else {
        at<u32>(window, 0x138) = 0;
        at<u32>(window, 0x13c) = 0;
    }
    Window705a20_SaveRect(window);
    at<u8 *>(window, 0x164) = WindowTimers_Add((u32)(uintptr_t)window, 0x2ee, 0x75e88);
    at<u16>(window, 0x14a) = 0xffff;
    at<u16>(window, 0x156) = 0;
    at<u16>(window, 0x154) = 0;
    at<u16>(window, 0x152) = 0;
    at<u16>(window, 0x150) = 0;
    at<u16>(window, 0x158) = 0;
    at<u16>(window, 0x14c) = 0xffff;
    at<u32>(window, 0x140) = 0;
    at<u32>(window, 0x144) = 0;
    at<u16>(window, 0x148) = 0;
    at<u16>(window, 0x14e) = 0;
    at<u32>(window, 0x15c) = 0;
    window[0x161] = 0;
    Window_SetHandler(window, nullptr, 0x66857d);
    at<u32>(window, 0x118) = 0x66857d;
    Window_SetFocus(window, nullptr, 0);
    window[0x160] = 1;
    window[0x162] = 0;
    if (*reinterpret_cast<u8 *>((uintptr_t)0x00faa56c)) window[0x162] = 1;
    return window;
}

void __fastcall Window705a20_SetCaret(u8 *window, void *, u8 *caret) {
    if (caret == at<u8 *>(window, 0x15c)) return;
    if (u8 *const old = at<u8 *>(window, 0x15c)) {
        if ((s16)WindowImpl_Release(old) == -1) vcall<u32>(old, 0, 1u);
    }
    at<u8 *>(window, 0x15c) = caret;
    if (!caret) return;
    Fn_6617f0(caret);
    u8 event[0x28];  // (only the words +4-+9, +0x14 (the rectangle) and +0x24 set)
    at<u16>(event, 4) = 0;
    at<u16>(event, 6) = (u16)(at<s16>(window, 0xe0) + 3);
    at<u16>(event, 8) = at<u16>(window, 0xe2);
    const s32 left = (s32)at<s16>(window, 0x30) + at<s16>(window, 0x150), top = (s32)at<s16>(window, 0x32) + at<s16>(window, 0x154);
    u8 rect[12];
    GuiRect_CtorWith(rect, nullptr, (u32)left, (u32)top, (u32)(left + 8), (u32)(top + at<s16>(window, 0x156)));
    at<u8 *>(event, 0x14) = rect;
    at<u32>(event, 0x24) = 0x17;
    vcall<void>(caret, 0xc, 0x17u, event);
    if ((s16)Window_IsShown(window) != 0) vcall<void>(at<u8 *>(window, 0x15c), 0x14, 1u);
    GuiRect_Dtor(rect);
}
}
