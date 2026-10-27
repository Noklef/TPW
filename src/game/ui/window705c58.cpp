#include "window705c58.h"

#include <cstdint>

#include "../core/vcall.h"
#include "window.h"

extern "C" {

u8 *__fastcall Window705c58_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    *reinterpret_cast<u32 *>(window) = 0x705c58;
    return window;
}

u32 __fastcall Window705c58_SetValue(u8 *window, void *, u32 value) {
    *reinterpret_cast<u32 *>(window + 0x13c) = value;
    u8 event[0x18];  // (only the word +4 set)
    *reinterpret_cast<u16 *>(event + 4) = (u16)value;
    if (u8 *const o = *reinterpret_cast<u8 **>(window + 0x98)) vcall<void>(o, 0xc, 1u, event);
    if (u8 *const o = *reinterpret_cast<u8 **>(window + 0x9c)) vcall<void>(o, 0xc, 1u, event);
    return Window_Notify80(window, nullptr, value);
}
}
