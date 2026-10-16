#include "lightlist.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall LightList_Clear(u8 *list) {
    at<u32>(list, 0x1c4) = 0;
    at<u32>(list, 0x1c0) = 0;
}

u8 *__fastcall LightList_First(u8 *list) {
    at<u32>(list, 0x1c4) = 0;
    return at<s32>(list, 0x1c0) > 0 ? list + 0x40 : nullptr;
}

u8 *__fastcall LightList_Next(u8 *list) {
    u32 cursor = at<u32>(list, 0x1c4) + 1;  // wraps, as the original's INC
    at<u32>(list, 0x1c4) = cursor;
    if ((s32)cursor >= at<s32>(list, 0x1c0))
        return nullptr;
    return list + ((cursor + 1) << 6);
}
}
