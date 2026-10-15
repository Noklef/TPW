#include "holder.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

}  // namespace

extern "C" {

u32 __cdecl Holder_IsAwake(const u8 *holder) { return at<u32>(holder, 0) & 1; }

void __cdecl Holder_FlagModel(const u8 *const *ref) { at<u32>(at<const u8 *>(*ref, 4), 0x30) |= 0x200; }

void __cdecl Holder_SetPosition(const u8 *holder, u32 x, u32 y, u32 z) {
    u8 *matrix = at<u8 *>(at<const u8 *>(holder, 4), 0x78) + 0x10;
    at<u32>(matrix, 0x30) = x;
    at<u32>(matrix, 0x34) = y;
    at<u32>(matrix, 0x38) = z;
}

u32 __cdecl Ride_SlotKey(u32 ride, u32 i) {
    const u8 *holder = at<const u8 *>(reinterpret_cast<const u8 *const *>(0x007a4610)[ride], 8);
    // Integer arithmetic: the original tests table + i * 0x14 for zero, which pointer
    // arithmetic on a null table would leave undefined.
    const uintptr_t slot = (uintptr_t)at<const u8 *>(at<const u8 *>(holder, 0x28), 4) + i * 0x14;
    return slot ? at<u32>(reinterpret_cast<const void *>(slot), 8) : 0;
}
}
