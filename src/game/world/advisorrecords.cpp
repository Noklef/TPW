#include "advisorrecords.h"

#include <cstdint>

namespace {
template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
}  // namespace

extern "C" {

u8 *__fastcall AdvisorSlot_Ctor(u8 *slot) {
    at<u8>(slot, 0x14) = 0;
    return slot;
}

u8 *__fastcall AdvisorEntry_Ctor(u8 *entry) {
    at<u32>(entry, 0) = 0;
    at<u32>(entry, 4) = 0;
    at<u32>(entry, 8) = 0;
    return entry;
}

u8 *__fastcall AdvisorRecord_Ctor(u8 *record) {
    at<u32>(record, 0) = 0;
    at<u32>(record, 4) = 0xffffffff;
    at<u8>(record, 8) = 0;
    at<u32>(record, 0xc) = 0;
    return record;
}

}  // extern "C"
