#include "window705b20.h"

#include <cstdint>

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Window705b20_SetRange(u8 *window, void *, s32 from, s32 to) {
    if (to == at<s32>(window, 0x138) && from == at<s32>(window, 0x13c)) return;
    s32 &step = at<s32>(window, 0x144);
    if (to < from) {
        if (step > 0) step = (s32)(0u - (u32)step);
    } else if (step < 0) {
        step = (s32)(0u - (u32)step);
    }
    at<s32>(window, 0x13c) = from;
    at<s32>(window, 0x138) = to;
}
}
