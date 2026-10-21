#include "window705b98.h"

#include <cstdint>

#include "window.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall Window705b98_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom) {
    Window_Ctor(window, nullptr, parent, flags, id, left, top, right, bottom);
    *reinterpret_cast<u32 *>(window) = 0x705b98;
    *reinterpret_cast<u32 *>(window + 0x138) = 0;
    return window;
}

u16 __fastcall Window705b98_SetBits(u8 *window, void *, u32 bits) {
    for (s32 i = 0; i < 0x1f; ++i) {
        const u32 bit = 1u << i;
        if (at<u32>(window, 0x138) & bit) {
            if (bits & bit) continue;
            if (u8 *const child = Window_FindChild(window, nullptr, (u32)i | 0x11100000)) Window_Notify1(child, nullptr, 0);
        } else if (bits & bit) {
            if (u8 *const child = Window_FindChild(window, nullptr, (u32)i | 0x11100000)) Window_Notify1(child, nullptr, 1);
        }
    }
    at<u32>(window, 0x138) = bits;
    Window_Notify80(window, nullptr, bits);
    return 1;
}
}
