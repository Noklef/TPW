#include "nav.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Nav_SetPosVel(u8 *nav, void *, u32 x, u32 y, u32 vx, u32 vy) {
    at<u32>(nav, 8) = x;
    at<u32>(nav, 0xc) = y;
    at<u32>(nav, 0x10) = vx;
    at<u32>(nav, 0x14) = vy;
}

void __fastcall Nav_GetPos(const u8 *nav, void *, u32 *x, u32 *y) {
    *x = at<u32>(nav, 8);
    *y = at<u32>(nav, 0xc);
}
}
