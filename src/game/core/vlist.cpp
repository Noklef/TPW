#include "vlist.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void Expire(u8 *object, u32 slot) {
    u8 *node = at<u8 *>(at<u8 *>(object, 0x18), 4);
    while (node) {
        u8 *next = at<u8 *>(node, 4);
        u8 *item = at<u8 *>(node, 0);
        if (vcall<u32>(item, 0x24))
            vcall<u32>(item, slot);
        node = next;
    }
}

}  // namespace

extern "C" {

void __stdcall Range14_CallSlotC(u8 *first, u8 *last) {
    for (u8 *element = first; element != last; element += 0x14)
        vcall<u32>(element, 0xc, 0u);
}

void __cdecl Range14_CallSlot18(u8 *first, u8 *last, u8 *value) {
    for (u8 *element = first; element != last; element += 0x14)
        vcall<u32>(element, 0x18, value);
}

u8 *__cdecl Range14_CopyBackward(u8 *first, u8 *last, u8 *destEnd) {
    u8 *dest = destEnd;
    while (first != last) {
        dest -= 0x14;
        last -= 0x14;
        vcall<u32>(dest, 0x18, last);
    }
    return dest;
}

void __fastcall List18_ExpireSlot8(u8 *object) { Expire(object, 8); }

void __fastcall List18_ExpireSlotC(u8 *object) { Expire(object, 0xc); }

u32 __fastcall List18_CallSlotC(u8 *object) {
    u8 *node = at<u8 *>(at<u8 *>(object, 0x18), 4);
    while (node) {
        u8 *next = at<u8 *>(node, 4);
        vcall<u32>(at<u8 *>(node, 0), 0xc);
        node = next;
    }
    return at<u32>(object, 0x14);
}

}  // extern "C"
