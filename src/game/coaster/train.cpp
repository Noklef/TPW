#include "train.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

}  // namespace

extern "C" {

void __cdecl Train_SetCouplings(u32, u8 *train) {
    if (*reinterpret_cast<const u32 *>(0x00790afc)) return;
    for (u32 i = 0; i < at<u32>(train, 0x30); ++i) {
        u8 *car = at<u8 *>(train, 0x34 + 4 * i);
        const u32 count = at<u32>(train, 0x30);
        u32 flags = at<u32>(car, 8);
        flags = count > 1 && i < count - 1 ? flags | 0x100 : flags & ~0x100u;
        at<u32>(car, 8) = flags;
        const u8 *type = at<const u8 *>(at<const u8 *>(car, 0x100), 0x18);
        const bool rear = !(flags & 0x2000) && (flags & 0x100) && (at<u8>(type, 0) & 8);
        at<u8 *>(car, 0x90) = car + (rear ? 0x84 : 0x78);
        at<u32>(car, 0x74) = at<u32>(at<const u8 *>(car, 0x100), 0x18) + (rear ? 0x44 : 0x30);
    }
}
}
