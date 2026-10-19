#include "window7059f0.h"

#include <cstdint>

#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"
#include "window705b20.h"
#include "windowfactory.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Window7059f0_Layout(u8 *window) {
    GuiRect_Assign(window + 0x2c, nullptr, window + 8);
    GuiRect_Assign(window + 0x138, nullptr, window + 0x2c);
}

u8 *__fastcall Window7059f0_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    GuiRect_Ctor(window + 0x138);
    at<u32>(window, 0) = 0x7059f0;
    u32 bar_flags = 0x10;
    at<u32>(window, 0x14c) = 0xffffffff;
    at<u32>(window, 0x144) = 0;
    at<u32>(window, 0x148) = 0;
    at<u32>(window, 0x174) = 0;
    at<u16>(window, 0x170) = 0;
    at<u16>(window, 0x172) = 0;
    at<u32>(window, 0x154) = 0;
    at<u32>(window, 0x150) = 0;
    at<u32>(window, 0x158) = 0;
    at<u32>(window, 0x160) = 0xffffffff;
    at<u16>(window, 0x164) = 0;
    at<u16>(window, 0x166) = 0;
    at<u32>(window, 0x16c) = 0;
    at<u16>(window, 0x168) = 0;
    at<u16>(window, 0x16a) = 0;
    Window7059f0_Layout(window);
    if (flags & 0x1000) {
        if (flags & 0x40) bar_flags |= 0x40;
        u8 *const bar = Window_CreateChild(window, nullptr, 3, bar_flags, 1, (u32)(at<s16>(window, 0x34) + 4), at<u16>(window, 0xe),
                                           at<u16>(window, 0x10), at<u16>(window, 0x12));
        if (bar) {
            vcall<void>(bar, 0x14, 0u);
            Window_SetShown(bar, nullptr, 1);
            Window705b20_SetRange(bar, nullptr, 0, 0);
        }
    }
    at<u32>(window, 0x114) = 0x665c35;
    at<u32>(window, 0x118) = at<u32>(window, 0x114);
    return window;
}
}
