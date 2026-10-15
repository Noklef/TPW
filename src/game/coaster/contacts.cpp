#include "contacts.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

const u32 kOverflow = 0x400000, kLost = 0x200000;

u32 *contacts(u8 *node) { return &at<u32>(node, 0xb4); }
u32 &count(u8 *node) { return at<u32>(node, 0xb0); }

// Takes node out of from's list, closing it up.
void drop(u8 *owner, u8 *from, const u8 *node) {
    u32 *list = contacts(from), *kept = list;
    u32 dropped = 0;
    for (u32 i = 0; i < count(from); ++i) {
        if (list[i] != (u32)(uintptr_t)node)
            *kept++ = list[i];
        else
            ++dropped;  // its slot is overwritten or zeroed below (the original zeroes it here too)
    }
    const u32 n = (u32)((s32)((uintptr_t)kept - (uintptr_t)list) >> 2);
    for (u32 i = n; i < count(from); ++i) list[i] = 0;
    count(from) = n;
    at<u32>(owner, 0x140) -= dropped;
    if (dropped && (at<u32>(from, 8) & kOverflow)) at<u32>(from, 8) |= kLost;
}

}  // namespace

extern "C" {

void __cdecl Contacts_Clear(u8 *owner, u8 *node) {
    if (at<u32>(node, 8) & kOverflow) {
        for (u8 *other = at<u8 *>(owner, 0x54); other; other = at<u8 *>(other, 0x64)) drop(owner, other, node);
        for (int i = 0; i < 8; ++i) contacts(node)[i] = 0;
        at<u32>(owner, 0x140) -= count(node);
        count(node) = 0;
        at<u32>(node, 8) &= ~kOverflow;
        return;
    }
    for (u32 i = 0; i < count(node); ++i) {
        drop(owner, reinterpret_cast<u8 *>(contacts(node)[i]), node);
        contacts(node)[i] = 0;
    }
    at<u32>(owner, 0x140) -= count(node);
    count(node) = 0;
}

void __cdecl Contacts_ClearAll(u8 *owner) {
    for (u8 *node = at<u8 *>(owner, 0x54); node; node = at<u8 *>(node, 0x64)) {
        Contacts_Clear(owner, node);
        at<u32>(node, 8) |= kLost;
    }
}

void __cdecl Contacts_ClearPair(u8 *node, u8 *owner) {
    Contacts_Clear(owner, node);
    if (u8 *partner = at<u8 *>(node, 0x68)) Contacts_Clear(owner, partner);
}

void __cdecl Contacts_MarkLostPair(u8 *node) {
    at<u32>(node, 8) |= kLost;
    if (u8 *partner = at<u8 *>(node, 0x68)) at<u32>(partner, 8) |= kLost;
}
}
