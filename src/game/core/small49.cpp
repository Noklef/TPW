#include "small49.h"

#include <cstdint>

#include "small18.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Panel_Reset(u8 *panel) {
    u8 *const target = panel + 0x50;
    u8 *pixel = at<u8 *>(target, 0xc);
    const u32 count = at<u32>(target, 0) * at<u32>(target, 4);
    for (u32 i = 0; i < count; ++i) {
        pixel[0] = 0xff;
        pixel[1] = 0xfb;
        pixel[2] = 0xf8;
        pixel[3] = 0xc6;
        pixel += 4;
    }
    Pixels_BlendOnto(panel + 0x60, nullptr, target, 0, 0);
    Pixels_BlendOnto(panel + 0x20, nullptr, target, 0x220, 0);
    Pixels_BlendOnto(panel + 0x30, nullptr, target, 0x1dd, 6);
    Pixels_BlendOnto(panel + 0x40, nullptr, target, 0x141, 0xda);
    Pixels_BlendOnto(panel + 0x10, nullptr, target, 4, 0x9f);
    Pixels_BlendOnto(panel, nullptr, target, 0xb6, 0);
}

}  // extern "C"
