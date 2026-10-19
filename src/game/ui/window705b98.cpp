#include "window705b98.h"

#include <cstdint>

#include "window.h"

extern "C" {

u8 *__fastcall Window705b98_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    *reinterpret_cast<u32 *>(window) = 0x705b98;
    *reinterpret_cast<u32 *>(window + 0x138) = 0;
    return window;
}
}
