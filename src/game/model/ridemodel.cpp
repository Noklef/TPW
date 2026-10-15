#include "ridemodel.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

}  // namespace

extern "C" {

void __cdecl Ride_MarkModel(const u32 *index) {
    u8 *const instance = reinterpret_cast<u8 *const *>(0x007a4610)[*index];
    at<u32>(at<u8 *>(at<u8 *>(instance, 8), 4), 0x30) |= 0x20;
    at<u32>(at<u8 *>(at<u8 *>(at<u8 *>(instance, 8), 4), 0x78), 0) |= 0x80800;
    at<u32>(instance, 4) |= 0x1000000;
}
}
