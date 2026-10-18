#include "helpbg.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../ui/window.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall HelpBg_Refresh(u8 *self) {
    if (at<u16>(self, 4) != 0)
        vcall<void>(self, 0);
}

u32 __fastcall Obj_Clear18_0c(u8 *object) {
    at<u32>(object, 0x18) = 0;
    at<u16>(object, 0xc) = 0;
    return 0;
}

s32 __cdecl HelpBg_WindowHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) vcall<u32>(reinterpret_cast<u8 *>(Window_DwordA0(window, nullptr, 0)), 0x3c);
    return Window_CallDefault(window, nullptr, message, a, b);
}
}  // extern "C"
