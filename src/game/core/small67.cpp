#include "small67.h"

#include <cstdint>

#include "../render/renderer.h"
#include "small58.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *Mouse() { return global<u8 *>(0x007b05d8); }

}  // namespace

extern "C" {

void __cdecl Holder_Destroy(u8 *holder) {
    if (at<u32>(holder, 0) & 1)
        Holder_Sleep(holder);
    u8 *owner = at<u8 *>(holder, 8);
    if (!owner)
        owner = holder;
    at<s32>(owner, 0x34) = at<s32>(owner, 0x34) - 1;
    if (u8 *part = at<u8 *>(holder, 0x1c)) {
        u8 *adjusted = part + at<s32>(at<u8 *>(part, 0), 4);
        vcall<u32>(adjusted, 0, 1u);
    }
    at<u32>(holder, 0x1c) = 0;
}

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

void __fastcall Class6fef78_Release(u8 *object) {
    at<u32>(object, 0) = 0x006fef78;
    u8 *second = at<u8 *>(object, 0xb0);
    at<u32>(object, 0xa8) = 0;
    at<u32>(object, 0xa4) = 0;
    if (second)
        comcall<u32>(second, 8);
    if (u8 *first = at<u8 *>(object, 0xac))
        comcall<u32>(first, 8);
    at<u32>(object, 0xac) = 0;
    at<u32>(object, 0xb0) = 0;
    at<u32>(object, 0) = 0x006fef90;
}

}  // extern "C"
