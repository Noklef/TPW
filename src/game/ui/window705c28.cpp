#include "window705c28.h"

#include <cstdint>

#include "../core/vcall.h"
#include "guirect.h"
#include "window.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall Window705c28_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    GuiRect_Ctor(window + 0x148);
    at<u32>(window, 0) = 0x705c28;
    at<u32>(window, 0x140) = 0;
    at<u32>(window, 0x13c) = 0;
    at<u32>(window, 0x138) = 0;
    GuiRect_Assign(window + 0x2c, nullptr, window + 8);
    GuiRect_Assign(window + 0x148, nullptr, window + 0x2c);
    return window;
}

u16 __fastcall Window705c28_SetValue(u8 *window, void *, s32 value) {
    if (value == at<s32>(window, 0x140)) return 0;
    at<s32>(window, 0x140) = value;
    if (at<s32>(window, 0x140) > 0x400) at<s32>(window, 0x140) = 0x400;
    else if (at<s32>(window, 0x140) < 0) at<s32>(window, 0x140) = 0;
    s32 event[6];   // (only +0 set)
    event[0] = at<s32>(window, 0x140);
    if (u8 *const o = at<u8 *>(window, 0x98)) vcall<void>(o, 0xc, 8u, event);
    if (u8 *const o = at<u8 *>(window, 0x9c)) vcall<void>(o, 0xc, 8u, event);
    return 1;
}

u32 __fastcall Window705c28_Dtor(u8 *window) {
    at<u32>(window, 0) = 0x705c28;
    GuiRect_Dtor(window + 0x148);
    return Window_Dtor(window);
}
}
