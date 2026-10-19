#include "window705af0.h"

#include <cstdint>

#include "guirect.h"
#include "window.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall Window705af0_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    at<u32>(window, 0) = 0x705af0;
    at<u32>(window, 0x138) = 0;
    at<u32>(window, 0x13c) = 1;
    at<u32>(window, 0x140) = 0x9708cd;
    at<u32>(window, 0x144) = 0;
    at<u32>(window, 0x148) = 1;
    window[0x154] = 1;
    GuiRect_Assign(window + 0x2c, nullptr, window + 8);
    Window_SetHandler(window, nullptr, 0x66aa61);
    return window;
}
}
