#include "lists.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__cdecl CoasterList_Append(u8 *list) {
    u8 *mesh = at<u8 *>(at<u8 *>(at<u8 *>(global<u8 *>(0x00792218), 0x38), 0x24), 4);
    u8 *item = global<u8 *>(0x0079c604);
    at<u32>(item, 8) = 0;
    if (!at<u8 *>(list, 8)) at<u8 *>(list, 8) = item;
    if (at<u8 *>(list, 0xc)) at<u8 *>(at<u8 *>(list, 0xc), 8) = item;
    at<u8 *>(item, 0xc) = at<u8 *>(list, 0xc);
    at<u8 *>(list, 0xc) = item;
    if (!at<u8 *>(list, 0x10)) {
        u8 *slot = at<u8 *>(at<u8 *>(mesh, 0x78), 0x6c) + (global<u32>(0x007925c8) << 4);
        at<u8 *>(list, 0x10) = slot;
        at<u8>(slot, 0xc) = list[0] & 1;
        ++at<u16>(mesh, 0x3c);
        ++at<u16>(at<u8 *>(mesh, 0x78), 0x5a);
        ++global<u32>(0x007925c8);
    }
    ++at<u32>(list, 4);
    global<u8 *>(0x0079c604) += 0x10;
    return item;
}
}
