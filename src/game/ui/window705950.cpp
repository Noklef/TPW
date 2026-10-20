#include "window705950.h"

#include <cstdint>

#include "../core/initializers.h"
#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"
#include "window705b20.h"
#include "windowfactory.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Window705950_Layout(u8 *window) {
    GuiRect_Assign(window + 0x2c, nullptr, window + 8);
    at<u16>(window, 0x34) -= 0x30;
    GuiRect_Assign(window + 0x138, nullptr, window + 0x2c);
    at<u16>(window, 0x13c) += 4;
    at<u16>(window, 0x140) -= 4;
    at<u16>(window, 0x13e) += 4;
    at<u16>(window, 0x142) -= 4;
}

u8 *__fastcall Window705950_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    GuiRect_Ctor(window + 0x138);
    at<u32>(window, 0) = 0x705950;
    u32 bar_flags = 0x10;
    at<u32>(window, 0x148) = 0xffffffff;
    at<u32>(window, 0x144) = 0;
    at<u16>(window, 0x154) = 0;
    at<u16>(window, 0x156) = 0;
    at<u32>(window, 0x150) = 0;
    at<u32>(window, 0x14c) = 0;
    Window705950_Layout(window);
    if (flags & 0x1000) {
        if (flags & 0x40) bar_flags |= 0x40;
        u8 *const bar = Window_CreateChild(window, nullptr, 3, bar_flags | 0x80, 1, (u32)(at<s16>(window, 0x34) + 4), at<u16>(window, 0xe),
                                           at<u16>(window, 0x10), at<u16>(window, 0x12));
        if (bar) {
            vcall<void>(bar, 0x14, 0u);
            Window_SetShown(bar, nullptr, 1);
            Window705b20_SetRange(bar, nullptr, 0, 0);
        }
    }
    Window_SetHandler(window, nullptr, 0x65bb6b);
    return window;
}

u32 __fastcall Window705950_SetInner(u8 *window, void *, u32 left, u32 top, u32 right, u32 bottom) {
    return Fn_659dac(window + 0x138, nullptr, (u16)left, (u16)top, (u16)right, (u16)bottom);
}

u32 __cdecl Window705950_Proc(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 0x10003: Window705950_TrackAt(window, nullptr, (u32)((s32)a >> 16), a & 0xffff); break;
    case 0x13: Window705950_OffsetInner(window, nullptr, (u16)a, (u16)b); break;
    case 0x11006: Window705950_ClickAt(window, nullptr, (u32)((s32)b >> 16), b & 0xffff); break;
    case 0x11007: Window705950_PostChosen(window); break;
    case 0x800: Window705950_Scrolled(window); break;
    default: break;
    }
    return Window_DefaultProc(window, message, a, b);
}

void __fastcall Window705950_OffsetInner(u8 *window, void *, u32 dx, u32 dy) { vcall<void>(window + 0x138, 8, (u32)(u16)dx, (u32)(u16)dy); }

void __fastcall Window705950_SelectAt(u8 *window, void *, u32, u32 y) {
    if (!at<s16>(window, 0x154)) return;
    s32 row = ((s32)(s16)y - (s32)at<s16>(window, 0x13e)) / (s32)at<s16>(window, 0x154);
    if (row < (s32)at<s16>(window, 0x156) && row < at<s32>(window, 0x14c)) row += at<s32>(window, 0x150);
    else row = -1;
    if (row == at<s32>(window, 0x148)) return;
    at<s32>(window, 0x148) = row;
    if (at<s32>(window, 0x148) >= 0) Window_PostMessage(at<u8 *>(window, 0x60), 0x201, at<u32>(window, 0x8c), at<u32>(window, 0x148));
    if (*reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
}

void __fastcall Window705950_TrackAt(u8 *window, void *, u32 x, u32 y) {
    if (!(at<u32>(window, 0x48) & 0x80)) return;
    if ((s16)vcall<u32>(window + 0x2c, 0, (u32)(u16)x, (u32)(u16)y) == 0) return;
    Window705950_SelectAt(window, nullptr, (u16)x, (u16)y);
}

void __fastcall Window705950_PostChosen(u8 *window) {
    if (at<s32>(window, 0x148) >= 0) Window_PostMessage(at<u8 *>(window, 0x60), 0x200, at<u32>(window, 0x8c), at<u32>(window, 0x148));
}

void __fastcall Window705950_ClickAt(u8 *window, void *, u32 x, u32 y) {
    if (at<u32>(window, 0x48) & 0x80) {
        if (at<s32>(window, 0x148) >= 0) Window_PostMessage(at<u8 *>(window, 0x60), 0x200, at<u32>(window, 0x8c), at<u32>(window, 0x148));
        return;
    }
    Window705950_SelectAt(window, nullptr, (u16)x, (u16)y);
}

void __fastcall Window705950_Scrolled(u8 *window) {
    u8 *const bar = Window_FindChild(window, nullptr, 1);
    if (!bar) return;
    const u32 first = vcall<u32>(bar, 0x20);
    if (first == at<u32>(window, 0x150)) return;
    at<u32>(window, 0x150) = first;
    if (*reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 3, 0, 0);
}
}
