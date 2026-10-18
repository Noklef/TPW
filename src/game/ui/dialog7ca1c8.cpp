#include "dialog7ca1c8.h"

#include <cstdint>

#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

using SetCheck = void(__fastcall *)(u8 *, void *, bool);

void setCheck(u8 *panel, u32 id, bool on) {
    u8 *const child = Window_FindChild(panel, nullptr, id);
    reinterpret_cast<SetCheck>(at<void **>(child, 0)[5])(child, nullptr, on);
}
}

extern "C" {

void __cdecl Dialog7ca1c8_SetChecks(u32 flags) {
    u8 *const panel = Window_FindChild(global<u8 *>(0x007ca1c8), nullptr, 0x18);
    setCheck(panel, 0x1f, (flags & 1) != 0);
    bool second = (flags & 2) != 0;
    if (second && at<u8>(global<u8 *>(0x007ca1d0), 0x50) == 0) second = false;
    setCheck(panel, 0x20, second);
    setCheck(panel, 0x1b, (flags & 4) != 0);
    setCheck(panel, 0x19, (flags & 8) != 0);
    setCheck(panel, 0x1a, (flags & 0x10) != 0);
    setCheck(panel, 0x1c, (flags & 0x20) != 0);
    setCheck(panel, 0x21, (flags & 0x40) != 0);
    setCheck(panel, 0x1e, (flags & 0x80) != 0);
    setCheck(panel, 0x1d, (flags & 0x100) != 0);
}

void __cdecl Dialog7ca1c8_SetChecksIfUp(u32 flags) {
    if (global<u8 *>(0x007ca1c8) && global<u32>(0x007ca1cc)) Dialog7ca1c8_SetChecks(flags);
}
}
