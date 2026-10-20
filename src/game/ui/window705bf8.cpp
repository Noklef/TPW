#include "window705bf8.h"

#include <cstdint>

#include "guirect.h"
#include "window.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall Window705bf8_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    at<u32>(window, 0) = 0x705bf8;
    at<u32>(window, 0x148) = 0;
    at<u32>(window, 0x138) = 0;
    at<u32>(window, 0x13c) = 0;
    at<u32>(window, 0x140) = 0;
    at<u32>(window, 0x144) = 0;
    at<u16>(window, 0x14c) = 0;
    at<u16>(window, 0x14e) = 0;
    GuiRect_Assign(window + 0x2c, nullptr, window + 8);
    Window_SetHandler(window, nullptr, 0x66d6f5);
    return window;
}
}
