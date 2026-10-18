#include "surfacepool.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, s32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

s32 __cdecl SurfacePool_SideIndex(s32 side) {
    switch (side) {
    case 1:
        return 8;
    case 2:
        return 7;
    case 4:
        return 6;
    case 8:
        return 5;
    case 0x10:
        return 4;
    case 0x20:
        return 3;
    case 0x40:
        return 2;
    case 0x80:
        return 1;
    case 0x100:
        return 0;
    default:
        return -1;
    }
}

u32 __fastcall SurfacePool_Next(u8 *pool, void *, s32 side, u32) {
    const s32 index = SurfacePool_SideIndex(side);
    const u32 turn = at<u32>(pool, 0x80 + 4 * index);
    at<u32>(pool, 0x80 + 4 * index) = (turn - 1) & 1;
    return at<u32>(pool, 8 * index + 4 * (s32)turn);
}
}
