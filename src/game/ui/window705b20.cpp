#include "window705b20.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/vcall.h"
#include "window.h"

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

u16 __fastcall Window705b20_SetEnabled(u8 *window, void *, u32 enabled) {
    const s16 e = (s16)enabled;
    const bool disabled = (at<u32>(window, 0x48) & 2) != 0;
    if (e != 0 ? !disabled : disabled) return 0;
    // Each child's slot 5 gets the word with EAX's upper half as it stands: the first the enable's (only its AX is modelled), the others
    // the child's own address's (65d56b's result). The callees read the word only.
    u32 arg = (Window_SetEnabled(window, nullptr, (u32)(s32)e) & 0xffff0000u) | (u16)e;
    for (u8 *child = at<u8 *>(window, 0x5c); child;) {
        vcall<void>(child, 0x14, arg);
        child = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(child));
        arg = ((u32)(uintptr_t)child & 0xffff0000u) | (u16)e;
    }
    if (*reinterpret_cast<u32 *>((uintptr_t)0x00faa5d4)) Window_PostMessage(window, 1, 0, 0);
    return 1;
}
}
