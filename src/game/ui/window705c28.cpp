#include "window705c28.h"

#include <cstdint>

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
}
