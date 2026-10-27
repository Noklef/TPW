#include "window705b20.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"
#include "windowfactory.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// a / divisor in one CDQ, IDIV, as the original: it faults (sic) on 0 or INT_MIN / -1.
s32 Idiv(s32 a, s32 divisor) {
    s32 q;
    __asm {
        mov eax, a
        cdq
        idiv divisor
        mov q, eax
    }
    return q;
}

}  // namespace

extern "C" {

void __fastcall Window705b20_SetRange(u8 *window, void *, s32 from, s32 to) {
    if (to == at<s32>(window, 0x138) && from == at<s32>(window, 0x13c)) return;
    s32 &step = at<s32>(window, 0x144);
    if (to < from) {
        if (step > 0) step = (s32)(0u - (u32)step);
    } else if (step < 0) {
        step = (s32)(0u - (u32)step);
    }
    at<s32>(window, 0x13c) = from;
    at<s32>(window, 0x138) = to;
}

u32 __fastcall Window705b20_SetPos(u8 *window, void *, s32 mode, s32 value) {
    u8 rect[12];
    GuiRect_Ctor(rect);
    u8 *const thumb = Window_FindChild(window, nullptr, 3);
    Window_GetRect(thumb, nullptr, rect, 1);
    const s32 range = (s32)(at<u32>(window, 0x138) - at<u32>(window, 0x13c));
    const s32 before = at<s32>(window, 0x140);
    s32 pos = 0;  // sic: the original leaves it unset for any other mode (no caller passes one)
    switch (mode) {
    case 1: pos = (s32)(at<u32>(window, 0x140) + at<u32>(window, 0x144)); break;
    case 0: pos = (s32)(at<u32>(window, 0x140) - at<u32>(window, 0x144)); break;
    case 2: pos = (s32)(at<u32>(window, 0x140) + at<u32>(window, 0x148)); break;
    case 3: pos = (s32)(at<u32>(window, 0x140) - at<u32>(window, 0x148)); break;
    case -1: pos = value; break;
    case -2: pos = at<s32>(window, 0x140); break;
    default: break;
    }
    if (at<s32>(window, 0x144) > 0) {
        if (pos > at<s32>(window, 0x138)) pos = at<s32>(window, 0x138);
        if (pos < at<s32>(window, 0x13c)) pos = at<s32>(window, 0x13c);
    } else {
        if (pos < at<s32>(window, 0x138)) pos = at<s32>(window, 0x138);
        if (pos > at<s32>(window, 0x13c)) pos = at<s32>(window, 0x13c);
    }
    if (pos == at<s32>(window, 0x140) && mode != -2) {
        GuiRect_Dtor(rect);
        return 0;
    }
    at<s32>(window, 0x140) = pos;
    if (at<s16>(window, 0x14c) & 2) {   // across: the thumb's left / right
        const s32 width = (s32)at<s16>(rect, 8) - at<s16>(rect, 4);
        const s32 room = (s32)at<s16>(window, 0x34) - width - at<s16>(window, 0x30);
        if (range != 0) {
            at<u16>(rect, 4) = (u16)(Idiv((s32)((at<u32>(window, 0x140) - at<u32>(window, 0x13c)) * (u32)room), range) + at<s16>(window, 0x30));
        } else {
            at<u16>(rect, 4) = at<u16>(window, 0x30);
        }
        at<u16>(rect, 8) = (u16)(at<s16>(rect, 4) + width);
    } else {   // down: the thumb's top / bottom, the maximum at the top
        const s32 height = (s32)at<s16>(rect, 0xa) - at<s16>(rect, 6);
        const s32 room = (s32)at<s16>(window, 0x36) - height - at<s16>(window, 0x32);
        if (range != 0) {
            at<u16>(rect, 6) = (u16)(Idiv((s32)((at<u32>(window, 0x138) - at<u32>(window, 0x140)) * (u32)room), range) + at<s16>(window, 0x32));
        } else {
            at<u16>(rect, 6) = at<u16>(window, 0x32);
        }
        at<u16>(rect, 0xa) = (u16)(at<s16>(rect, 6) + height);
    }
    Window_SetRect(thumb, nullptr, rect);
    const s32 range2 = (s32)(at<u32>(window, 0x138) - at<u32>(window, 0x13c));
    if (range2 != 0) {
        u8 event[0x18];  // (only the dword +0 set: the position in 1024ths, its low word sign-extended)
        at<s32>(event, 0) = (s16)Idiv((s32)((at<u32>(window, 0x140) - at<u32>(window, 0x13c)) << 10), range2);
        if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 8u, event);
        if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 8u, event);
    }
    Window_Notify80(window, nullptr, at<u32>(window, 0x140));
    if (before != at<s32>(window, 0x140)) {
        Window_SendMessage(at<u8 *>(window, 0x60), 0x800, at<u32>(window, 0x8c), at<u32>(window, 0x140));
    }
    GuiRect_Dtor(rect);
    return 1;
}

void __fastcall Window705b20_PageToward(u8 *window, void *, u32 x, u32 y) {
    u8 *const thumb = Window_FindChild(window, nullptr, 3);
    if (!thumb) return;
    u8 rect[12];
    GuiRect_Ctor(rect);
    Window_GetRect(thumb, nullptr, rect, 1);
    const s32 left = at<s16>(rect, 4), top = at<s16>(rect, 6), right = at<s16>(rect, 8), bottom = at<s16>(rect, 0xa);
    const s16 d = (at<s16>(window, 0x14c) & 2) ? (s16)((s16)x - (left + (right - left))) : (s16)(top + (bottom - top) - (s16)y);
    s32 mode = d < 0 ? 2 : 3;
    if (at<s32>(window, 0x144) > 0) mode = mode == 2 ? 3 : 2;
    Window705b20_SetPos(window, nullptr, mode, 0);
    GuiRect_Dtor(rect);
}

u32 __cdecl Window705b20_MoveInside(u8 *rect, GuiRectByValue bounds, u32 dx, u32 dy, u32 across) {
    const u32 lo = across ? 4 : 6, hi = lo + 4;
    const u16 d = (u16)(across ? dx : dy);
    at<u16>(rect, lo) = (u16)(at<u16>(rect, lo) + d);
    at<u16>(rect, hi) = (u16)(at<u16>(rect, hi) + d);
    const s32 size = (s32)at<s16>(rect, hi) - at<s16>(rect, lo);
    if (at<s16>(rect, lo) < at<s16>(bounds.bytes, lo)) {
        at<s16>(rect, lo) = at<s16>(bounds.bytes, lo);
        at<u16>(rect, hi) = (u16)(at<s16>(rect, lo) + size);
    }
    if (at<s16>(rect, hi) > at<s16>(bounds.bytes, hi)) {
        at<s16>(rect, hi) = at<s16>(bounds.bytes, hi);
        at<u16>(rect, lo) = (u16)(at<s16>(rect, hi) - size);
    }
    return GuiRect_Dtor(bounds.bytes);
}

u32 __cdecl Window705b20_Proc(u8 *bar, u32 message, u32 a, u32 b) {
    switch (message) {
    case 3:
        vcall<u32>(bar, 0x10);
        if (u8 *const thumb = Window_FindChild(bar, nullptr, 3)) Window_OpenTree(thumb);
        return 0;
    case 0x10008:
    case 0x10009:
    case 0x100:
        if (a == 1) {
            Window705b20_SetPos(bar, nullptr, 1, 0);
        } else if (a == 2) {
            Window705b20_SetPos(bar, nullptr, 0, 0);
        }
        break;
    case 0x1000d: vcall<u32>(bar, 0x1c, vcall<u32>(bar, 0x20) - a); break;
    case 0x11006: Window705b20_PageToward(bar, nullptr, (u32)((s32)b >> 16), b & 0xffff); break;
    case 0xffffffffu:
        if (*reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4)) {
            Window_PostMessage(Window_FindChild(bar, nullptr, 3), 1, 0, 0);
            vcall<u32>(bar, 0x10);
        }
        break;
    default: break;
    }
    return Window_DefaultProc(bar, message, a, b);
}

void __fastcall Window705b20_PosFromThumb(u8 *window) {
    u8 rect[12];
    GuiRect_Ctor(rect);
    u8 *const thumb = Window_FindChild(window, nullptr, 3);
    Window_GetRect(thumb, nullptr, rect, 1);
    const s32 range = (s32)(at<u32>(window, 0x138) - at<u32>(window, 0x13c));
    const s32 before = at<s32>(window, 0x140);
    if (at<s16>(window, 0x14c) & 2) {   // across: from the left end +0x30
        const s32 width = (s32)at<s16>(rect, 8) - at<s16>(rect, 4);
        const s32 room = (s32)at<s16>(window, 0x34) - width - at<s16>(window, 0x30);
        const s32 off = (s32)at<s16>(rect, 4) - at<s16>(window, 0x30);
        at<u32>(window, 0x140) = (u32)Idiv((s32)((u32)off * (u32)range), room) + at<u32>(window, 0x13c);
    } else {   // down: from the bottom end +0x36
        const s32 height = (s32)at<s16>(rect, 0xa) - at<s16>(rect, 6);
        const s32 room = (s32)at<s16>(window, 0x36) - height - at<s16>(window, 0x32);
        const s32 off = (s32)at<s16>(window, 0x36) - at<s16>(rect, 0xa);
        at<u32>(window, 0x140) = (u32)Idiv((s32)((u32)off * (u32)range), room) + at<u32>(window, 0x13c);
    }
    if (before != at<s32>(window, 0x140)) {
        Window_PostMessage(at<u8 *>(window, 0x60), 0x800, at<u32>(window, 0x8c), at<u32>(window, 0x140));
    }
    GuiRect_Dtor(rect);
}

u16 __fastcall Window705b20_SetEnabled(u8 *window, void *, u32 enabled) {
    const s16 e = (s16)enabled;
    const bool disabled = (at<u32>(window, 0x48) & 2) != 0;
    if (e != 0 ? !disabled : disabled) return 0;
    // Each child's slot 5 gets the word with EAX's upper half as it stands: the first the enable's (only its AX is modelled), the others
    // the child's own address's (65d56b's result). The callees read the word only.
    u32 arg = (Window_SetEnabled(window, nullptr, (u32)(s32)e) & 0xffff0000u) | (u16)e;
    for (u8 *child = at<u8 *>(window, 0x5c); child;) {
        vcall<void>(child, 0x14, arg);
        child = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(child));
        arg = ((u32)(uintptr_t)child & 0xffff0000u) | (u16)e;
    }
    if (*reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 1, 0, 0);
    return 1;
}

void __fastcall Window705b20_Layout(u8 *window) {
    const u16 flags = at<u16>(window, 0x14c);
    if (flags & 4) {
        GuiRect_Assign(window + 0x2c, nullptr, window + 8);
    } else if (flags & 2) {
        at<u16>(window, 0x32) = at<u16>(window, 0xe);
        at<u16>(window, 0x36) = at<u16>(window, 0x12);
        const s16 d = (s16)(at<s16>(window, 0x36) - at<s16>(window, 0x32));
        at<u16>(window, 0x30) = (u16)(at<s16>(window, 0xc) + d);
        at<u16>(window, 0x34) = (u16)(at<s16>(window, 0x10) - d);
    } else {
        at<u16>(window, 0x30) = at<u16>(window, 0xc);
        at<u16>(window, 0x34) = at<u16>(window, 0x10);
        const s16 d = (s16)(at<s16>(window, 0x34) - at<s16>(window, 0x30));
        at<u16>(window, 0x32) = (u16)(at<s16>(window, 0xe) + d);
        at<u16>(window, 0x36) = (u16)(at<s16>(window, 0x12) - d);
    }
}

void __fastcall Window705b20_AddArrow1(u8 *window) {
    const bool flag2 = (at<u16>(window, 0x14c) & 2) != 0;
    u8 *const child =
        Window_CreateChild(window, nullptr, 2, 0, 1, at<u16>(window, flag2 ? 0x34 : 0x30), at<u16>(window, flag2 ? 0x32 : 0xe),
                           at<u16>(window, flag2 ? 0x10 : 0x34), at<u16>(window, flag2 ? 0x36 : 0x32));
    if (child) Window_SetShown(child, nullptr, 1);
}

void __fastcall Window705b20_AddArrow2(u8 *window) {
    const bool flag2 = (at<u16>(window, 0x14c) & 2) != 0;
    u8 *const child =
        Window_CreateChild(window, nullptr, 2, 0, 2, at<u16>(window, flag2 ? 0xc : 0x30), at<u16>(window, flag2 ? 0x32 : 0x36),
                           at<u16>(window, flag2 ? 0x30 : 0x34), at<u16>(window, flag2 ? 0x36 : 0x12));
    if (child) Window_SetShown(child, nullptr, 1);
}

void __fastcall Window705b20_AddThumb(u8 *window) {
    const bool flag2 = (at<u16>(window, 0x14c) & 2) != 0;
    const u16 right = flag2 ? (u16)(at<s16>(window, 0x30) + 0x40) : at<u16>(window, 0x34);
    const u16 bottom = flag2 ? at<u16>(window, 0x36) : (u16)(at<s16>(window, 0x32) + 0x40);
    u8 *const child = Window_CreateChild(window, nullptr, 2, 0, 3, at<u16>(window, 0x30), at<u16>(window, 0x32), right, bottom);
    if (!child) return;
    Window_SetShown(child, nullptr, 1);
    Window_SetHandler(child, nullptr, 0x66bb9b);
}

u8 *__fastcall Window705b20_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    at<u32>(window, 0) = 0x705b20;
    at<u32>(window, 0x138) = 0;
    at<u32>(window, 0x13c) = 0;
    at<u32>(window, 0x140) = 0;
    at<u32>(window, 0x144) = 1;
    at<u16>(window, 0x14c) = (flags & 0x10) ? 1 : 2;
    if (flags & 0x40) at<u16>(window, 0x14c) |= 4;
    at<u32>(window, 0x148) = 1;
    Window705b20_Layout(window);
    if (!(at<u16>(window, 0x14c) & 4) && (flags & 0x80)) {
        Window705b20_AddArrow1(window);
        Window705b20_AddArrow2(window);
    }
    if (flags & 0x80) Window705b20_AddThumb(window);
    at<u32>(window, 0x114) = 0x66ba22;
    at<u32>(window, 0x118) = at<u32>(window, 0x114);
    return window;
}

void __fastcall Window705b20_NotifyScroll(u8 *window) {
    Window_SendMessage(at<u8 *>(window, 0x60), 0x801, at<u32>(window, 0x8c), at<u32>(window, 0x140));
}

void __fastcall Window705b20_NotifyScrollEnd(u8 *window) {
    Window_SendMessage(at<u8 *>(window, 0x60), 0x802, at<u32>(window, 0x8c), at<u32>(window, 0x140));
}
}
