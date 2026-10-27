#include "waveformat.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall WaveFormat_Build(u8 *holder) {
    if (at<u16>(holder, 4)) return holder + 4;
    if (const u8 *const record = at<const u8 *>(holder, 0x18)) {
        at<u16>(holder, 6) = (record[0x1b] & 1) ? 2 : 1;
        at<u32>(holder, 8) = at<u16>(record, 0x18);
        at<u16>(holder, 0x12) = record[0x1a];
    } else {
        at<u16>(holder, 6) = holder[0];
        at<u16>(holder, 0x12) = holder[1];
        at<u32>(holder, 8) = at<u16>(holder, 2);
    }
    const u32 align = (static_cast<u32>(at<u16>(holder, 6)) * at<u16>(holder, 0x12)) >> 3;   // (bytes: under 0x2000)
    at<u16>(holder, 4) = 1;
    at<u16>(holder, 0x10) = static_cast<u16>(align);
    at<u32>(holder, 0xc) = at<u32>(holder, 8) * align;
    at<u16>(holder, 0x14) = 0;
    return holder + 4;
}

}  // extern "C"
