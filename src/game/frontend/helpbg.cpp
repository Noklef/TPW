#include "helpbg.h"

#include <cstdint>

#include "../core/vcall.h"

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
}  // extern "C"
