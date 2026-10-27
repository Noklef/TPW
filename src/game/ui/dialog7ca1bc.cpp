#include "dialog7ca1bc.h"

#include <cstdint>

#include "window.h"
#include "windowscript.h"

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
}

extern "C" {

void __cdecl Dialog7ca1bc_Create() {
    global<u8 *>(0x007ca1bc) = WindowScript_Build(global<u8 *>(0x00faa5d8), nullptr, reinterpret_cast<u8 *>(0x007514c0), 0x00497a70);
    Window_Unlink(global<u8 *>(0x007ca1bc));
    Window_SetDwordA0(global<u8 *>(0x007ca1bc), nullptr, 0, 0);
    Window_SetShown(global<u8 *>(0x007ca1bc), nullptr, 0);
}
}
