#include "userslots.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall Records24_CountUsed(const u8 *records) {
    u32 count = 0;
    for (u32 i = 0; i < 4; i++)
        count += at<u32>(records, 4 + 24 * i) != 0 ? 1 : 0;
    return count;
}

u8 *__fastcall Records24_IfUsed(u8 *records, void *, u32 i) {
    u8 *record = records + 24 * i;
    return at<u32>(record, 4) != 0 ? record : nullptr;
}

u32 __fastcall Records24_Field14(const u8 *records, void *, u32 i) { return at<u32>(records, 24 * i + 0x14); }

}  // extern "C"
