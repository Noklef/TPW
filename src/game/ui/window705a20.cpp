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

u32 __fastcall Window705a20_Dtor(u8 *window) {
    at<u32>(window, 0) = 0x705a20;
    if (at<u32>(window, 0x138) && (at<u32>(window, 0x48) & 0x40)) GameHeap_Free(at<void *>(window, 0x138));
    if (at<u32>(window, 0x164)) WindowTimers_Kill((u32)(uintptr_t)window, 0x75e88);
    if (u8 *const caret = at<u8 *>(window, 0x15c)) {
        if ((s16)WindowImpl_Release(caret) == -1) vcall<u32>(caret, 0, 1u);
    }
    return Window_Dtor(window);
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

void __fastcall Window705a20_MoveCaret(u8 *window, void *, u32, u32) {
    u8 *const caret = at<u8 *>(window, 0x15c);
    if (!caret) return;
    const s32 left = (s32)at<s16>(window, 0x30) + at<s16>(window, 0x150), top = (s32)at<s16>(window, 0x32) + at<s16>(window, 0x154);
    u8 rect[12], event[0x18];  // (only the event's +0x14 set: the rectangle)
    GuiRect_CtorWith(rect, nullptr, (u32)left, (u32)top, (u32)(left + 8), (u32)(top + at<s16>(window, 0x156)));
    at<u8 *>(event, 0x14) = rect;
    vcall<void>(caret, 0xc, 0x10u, event);
    GuiRect_Dtor(rect);
}

u32 __fastcall Window705a20_KeyToParent(u8 *window, void *, u32 key, u32 value) {
    u8 *const parent = at<u8 *>(window, 0x60);
    const u32 id = at<u32>(window, 0x8c);
    switch ((s16)key) {
    case 9: return (u32)Window_SendMessage(parent, 0x803, id, value);
    case 0xd:
        if (at<u32>(window, 0x48) & 0x80) return (u32)(uintptr_t)window;  // sic: EAX the window
        return (u32)Window_SendMessage(parent, 0x802, id, value);
    case 0x1b: return (u32)Window_SendMessage(parent, 0x804, id, value);
    default: return (u32)Window_SendMessage(parent, 0x806, id, (u32)(s32)(s16)key);
    }
}

void __fastcall Window705a20_ToggleInsert(u8 *window) {
    if (at<s16>(window, 0x14c) != -1 || at<s16>(window, 0x158) == -1) return;
    at<u16>(window, 0x158) ^= 1;
    if (at<u8 *>(window, 0x15c)) {
        // (the word pushed with the window's upper half: AX loaded over EAX still holding the window)
        vcall<void>(at<u8 *>(window, 0x15c), 0x14, ((u32)(uintptr_t)window & 0xffff0000u) | at<u16>(window, 0x158));
    }
    if (*reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
}

void __fastcall Window705a20_NotifyText(u8 *window, void *, u32 post) {
    u8 text[0x10], event[0x18];  // (only the text's dword +0 and word +0xa and the event's +0x10 set)
    at<u32>(text, 0) = at<u32>(window, 0x138);
    at<u16>(text, 0xa) = at<u16>(window, 0x148);
    at<u8 *>(event, 0x10) = text;
    if (at<u8 *>(window, 0x9c)) vcall<void>(at<u8 *>(window, 0x9c), 0xc, 0x4100u, event);
    if (post) Window_PostMessage(at<u8 *>(window, 0x60), 0x801, at<u32>(window, 0x8c), 0);
}

void __fastcall Window705a20_SetInsertMode(u8 *window, void *edx, u32 mode) {
    const bool none = (s16)mode == 0;
    at<u16>(window, 0x158) = none ? 0xffff : (u16)mode;
    if (at<u8 *>(window, 0x15c)) {
        // (the word pushed with the upper half of EDX: the caller's without a mode, else the window's address's; sic)
        const u32 high = (u32)(uintptr_t)(none ? edx : window) & 0xffff0000u;
        vcall<void>(at<u8 *>(window, 0x15c), 0x14, high | (mode & 0xffff));
    }
    if (*reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
}
}
