#include "backdrop.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Backdrop_Clear5c(u8 *backdrop) { at<u32>(backdrop, 0x5c) = 0; }

void __fastcall Backdrop_SetRect(u8 *backdrop, void *, u32 x0, u32 y0, u32 x1, u32 y1) {
    at<u32>(backdrop, 0) |= 2;
    at<u32>(backdrop, 0x20) = x0;
    at<u32>(backdrop, 0x24) = y0;
    at<u32>(backdrop, 0x28) = x1;
    at<u32>(backdrop, 0x2c) = y1;
}

void __fastcall Backdrop_SetSourceRect(u8 *backdrop, void *, u32 x0, u32 y0, u32 x1, u32 y1) {
    at<u32>(backdrop, 0x30) = x0;
    at<u32>(backdrop, 0x34) = y0;
    at<u32>(backdrop, 0x38) = x1;
    at<u32>(backdrop, 0x3c) = y1;
}
}
