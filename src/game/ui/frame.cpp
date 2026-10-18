#include "frame.h"

#include <cstdint>

#include "guimanager.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Frame_SetInterval(u8 *frame, void *, u32 value) {
    at<u32>(frame, 0x14) = value;
    if (at<s16>(frame, 4) != 0 && at<s16>(frame, 0xc) != 0) at<u32>(frame, 0x10) = GuiClock_Now();
}
}  // extern "C"
