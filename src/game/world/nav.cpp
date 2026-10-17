#include "nav.h"

#include <cstdint>
#include "../core/small20.h"
#include "../math/intpair.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

namespace {

// The words cleared before the elements are built (the others written then are overwritten later).
const u32 kZeroed[] = {0x8, 0xc, 0x10, 0x14, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c, 0x40, 0x44, 0x4c, 0x50, 0x54, 0x58, 0x5c};

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
u8 *__fastcall Nav_Ctor(u8 *object) {
    at<u32>(object, 0) = 1;
    at<u32>(object, 4) = 0xa;
    for (u32 offset : kZeroed)
        at<u32>(object, offset) = 0;
    at<u32>(object, 0x18) = 0x10000;
    at<u32>(object, 0x1c) = 0x50000;
    at<u8>(object, 0x60) = 1;
    CRT_VectorCtorIterator(object + 0x64, 8, 5, reinterpret_cast<void(__fastcall *)(void *)>(IntPair_Ctor), reinterpret_cast<void *>(0x005d1750));
    for (u32 offset = 0xa0; offset <= 0xb8; offset += 4)
        at<u32>(object, offset) = 0;
    at<u32>(object, 4) = 0x3333;
    at<u32>(object, 0) = 0x10000;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x1c) = 0x3333;
    at<u32>(object, 0x18) = 0x6666;
    return object;
}

}
