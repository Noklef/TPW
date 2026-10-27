#include "window705bc8.h"

#include <cstdint>

#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall Window705bc8_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    at<u32>(window, 0) = 0x705bc8;
    at<u32>(window, 0x144) = 0;
    at<u32>(window, 0x148) = 0;
    at<u32>(window, 0x138) = 0;
    at<u32>(window, 0x13c) = 0xfff;
    at<u32>(window, 0x14c) = 0;
    at<u32>(window, 0x150) = 1;
    at<u32>(window, 0x158) = 0;
    at<u32>(window, 0x15c) = 0;
    at<u32>(window, 0x140) = at<u32>(window, 0x13c) - at<u32>(window, 0x138);
    at<u32>(window, 0x154) = at<u32>(window, 0x150) - at<u32>(window, 0x14c);
    GuiRect_Assign(window + 0x2c, nullptr, window + 8);
    at<u16>(window, 0x164) = 0;
    Window_SetHandler(window, nullptr, 0x66d001);
    return window;
}

u8 *__fastcall Window705bc8_SetRange(u8 *window, void *, const s32 *low, const s32 *high) {
    if (low) at<s32>(window, 0x138) = *low;
    if (high) at<s32>(window, 0x13c) = *high;
    if (at<s32>(window, 0x138) > at<s32>(window, 0x13c)) {
        at<s32>(window, 0x138) = at<s32>(window, 0x13c);
        at<s32>(window, 0x13c) = at<s32>(window, 0x138);  // sic
    }
    at<s32>(window, 0x140) = at<s32>(window, 0x13c) - at<s32>(window, 0x138);
    return window;
}

u8 *__fastcall Window705bc8_SetRange2(u8 *window, void *, const s32 *low, const s32 *high) {
    if (low) at<s32>(window, 0x14c) = *low;
    if (high) at<s32>(window, 0x150) = *high;
    if (at<s32>(window, 0x14c) > at<s32>(window, 0x150)) {
        at<s32>(window, 0x14c) = at<s32>(window, 0x150);
        at<s32>(window, 0x150) = at<s32>(window, 0x14c);  // sic
    }
    if (at<s32>(window, 0x158) < at<s32>(window, 0x14c)) at<s32>(window, 0x158) = at<s32>(window, 0x14c);
    else if (at<s32>(window, 0x158) > at<s32>(window, 0x150)) at<s32>(window, 0x158) = at<s32>(window, 0x150);
    at<s32>(window, 0x154) = at<s32>(window, 0x150) - at<s32>(window, 0x14c);
    return window;
}

s32 __fastcall Window705bc8_FirstToSecond(u8 *window) {
    if (at<s32>(window, 0x144) == at<s32>(window, 0x13c)) return at<s32>(window, 0x150);
    if (at<s32>(window, 0x144) == at<s32>(window, 0x138)) return at<s32>(window, 0x14c);
    const s32 product = (s32)(at<u32>(window, 0x154) * at<u32>(window, 0x144));   // (wrapping, as IMUL)
    return (s32)((u32)(product / at<s32>(window, 0x140)) + at<u32>(window, 0x14c));
}

s32 __fastcall Window705bc8_SecondToFirst(u8 *window) {
    if (at<s32>(window, 0x158) == at<s32>(window, 0x150)) return at<s32>(window, 0x13c);
    if (at<s32>(window, 0x158) == at<s32>(window, 0x14c)) return at<s32>(window, 0x138);
    const s32 product = (s32)((at<u32>(window, 0x158) - at<u32>(window, 0x14c)) * at<u32>(window, 0x140));   // (wrapping, as IMUL)
    return (s32)((u32)(product / at<s32>(window, 0x154)) + at<u32>(window, 0x138));
}

s32 __fastcall Window705bc8_Compare(u8 *window, void *, u8 *other) {
    const u32 theirs = vcall<u32>(other, 0xc);
    if (theirs != vcall<u32>(window, 0xc)) return 0;
    return (s32)(at<u32>(window, 0x158) - at<u32>(other, 0x158));
}

void __fastcall Window705bc8_SetValue(u8 *window, void *, s32 value) {
    if (value > at<s32>(window, 0x150))
        value = at<s32>(window, 0x150);
    else if (value < at<s32>(window, 0x14c))
        value = at<s32>(window, 0x14c);
    if (value == at<s32>(window, 0x158)) return;
    at<s32>(window, 0x15c) = at<s32>(window, 0x158);
    at<s32>(window, 0x158) = value;
    at<s32>(window, 0x148) = Window705bc8_SecondToFirst(window);
    if (u8 *const object = at<u8 *>(window, 0x9c)) {
        u8 event[0x18] = {};
        *reinterpret_cast<u16 *>(event + 8) = (u16)at<u32>(window, 0x148);
        vcall<void>(object, 0xc, 4u, event);
    }
    Window_PostMessage(at<u8 *>(window, 0x60), 0x800, at<u32>(window, 0x8c), at<u32>(window, 0x158));
}
u32 __fastcall Window705bc8_SetValueSync(u8 *window, void *, s32 value) {
    Window705bc8_SetValue(window, nullptr, value);
    at<s32>(window, 0x144) = at<s32>(window, 0x148);
    return (at<u32>(window, 0x148) & 0xffff0000u) | 1;
}

void __fastcall Window705bc8_SetFirst(u8 *window, void *, s32 value) {
    if (value < at<s32>(window, 0x138))
        value = at<s32>(window, 0x138);
    else if (value > at<s32>(window, 0x13c))
        value = at<s32>(window, 0x13c);
    if (value == at<s32>(window, 0x144)) return;
    at<s32>(window, 0x144) = value;
    Window705bc8_SetValue(window, nullptr, Window705bc8_FirstToSecond(window));
}

void __fastcall Window705bc8_SetDrag(u8 *window, void *, u32 on, u32 y) {
    if ((s16)on) {
        at<u16>(window, 0x164) = 1;
        at<u16>(window, 0x160) = (u16)y;
        at<u16>(window, 0x162) = at<u16>(window, 0x160);
        Window_CaptureMouse(window);
    } else {
        at<u16>(window, 0x164) = 0;
        Window_ReleaseMouse(window);
    }
}

void __fastcall Window705bc8_Drag(u8 *window, void *, u32 y) {
    if (!at<s16>(window, 0x164) || (y & 0xffff) == (u32)(s32)at<s16>(window, 0x160)) return;   // (the word unsigned against the signed one, sic)
    at<u16>(window, 0x162) = at<u16>(window, 0x160);
    at<u16>(window, 0x160) = (u16)y;
    const s32 moved = (s32)at<s16>(window, 0x162) - (s32)at<s16>(window, 0x160);
    Window705bc8_SetFirst(window, nullptr, (s32)(at<u32>(window, 0x144) + (u32)moved * 8));
}

u32 __cdecl Window705bc8_OnMessage(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x10005) {
        Window_Activate(window);
        if (!a) Window705bc8_SetDrag(window, nullptr, 1, b);
        return 0;
    }
    if (message == 0x10004) {
        if (!a) Window705bc8_SetDrag(window, nullptr, 0, 0);
        return 0;
    }
    if (message == 0x10003) Window705bc8_Drag(window, nullptr, a);
    return Window_DefaultProc(window, message, a, b);
}
}
