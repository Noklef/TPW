#include "mouse.h"

#include <cstdint>

#include "../core/vcall.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *Mouse() { return global<u8 *>(0x007b05d8); }

}  // namespace

extern "C" {

u32 __cdecl Mouse_HandleButton(u32 event, u32 state, u32 a, u32 b) {
    if (event == 0xf0) {
        if (state == 1) {
            vcall<u32>(Mouse(), 4, a, b);
            global<u32>(0x007b05e4) = 1;
        } else {
            vcall<u32>(Mouse(), 8, a, b);
            global<u32>(0x007b05e4) = 0;
        }
        return 1;
    }
    if (event == 0xf1) {
        if (state == 1) {
            vcall<u32>(Mouse(), 0xc, a, b);
            global<u32>(0x007b05f0) = 1;
        } else {
            vcall<u32>(Mouse(), 0x10, a, b);
            global<u32>(0x007b05f0) = 0;
        }
    }
    return 1;
}

void __stdcall Mouse_Moved(s32 x, s32 y) {
    if (x != global<s32>(0x007b05d4) || y != global<s32>(0x007b05e0))
        vcall<u32>(Mouse(), 0x18, (u32)x, (u32)y, global<u32>(0x007b05e4), global<u32>(0x007b05f0));
    if (global<u32>(0x007b05e4) != 0)
        vcall<u32>(Mouse(), 0x1c, (u32)x, (u32)y);
    else if (global<u32>(0x007b05f0) != 0)
        vcall<u32>(Mouse(), 0x20, (u32)x, (u32)y);
    global<s32>(0x007b05d4) = x;
    global<s32>(0x007b05e0) = x;  // (sic)
}

}  // extern "C"
