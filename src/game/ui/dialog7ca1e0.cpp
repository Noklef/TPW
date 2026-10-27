#include "dialog7ca1e0.h"

#include <cstdint>

#include "window.h"
#include "windowscript.h"

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
}

extern "C" {

void __cdecl Dialog7ca1e0_Create() {
    global<u8 *>(0x007ca1e0) = WindowScript_Build(global<u8 *>(0x00faa5d8), nullptr, reinterpret_cast<u8 *>(0x00751720), 0x00498ad0);
    Window_Unlink(global<u8 *>(0x007ca1e0));
    Window_SetDwordA0(global<u8 *>(0x007ca1e0), nullptr, 0, 1);
    Window_SetShown(global<u8 *>(0x007ca1e0), nullptr, 0);
}
}
