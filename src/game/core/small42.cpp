#include "small42.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *Record(s32 index) { return global<u8 *>(0x0080cef8) + index * 0x34; }

}  // namespace

extern "C" {

s32 __cdecl FreeList_Take(s32 index) {
    const s16 head = global<s16>(0x00816d24);
    if (head < 0) return -1;
    s16 *const slot = reinterpret_cast<s16 *>(0x0080cf02 + index * 0x140);
    const s16 old = *slot;
    *slot = head;
    const s16 next = at<s16>(Record(global<s16>(0x00816d24)), 0x30);
    global<s16>(0x00816d24) = next;
    if (next >= 0) at<u16>(Record(next), 0x32) = 0xffff;
    at<s16>(Record(*slot), 0x30) = old;
    if (old >= 0) at<s16>(Record(old), 0x32) = *slot;
    return *slot;
}

}  // extern "C"
