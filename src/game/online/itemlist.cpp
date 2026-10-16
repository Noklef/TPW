#include "itemlist.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The first node of the owner's list.
u8 *First(const u8 *owner) { return at<u8 *>(at<u8 *>(owner, 0x2c), 4); }

template <typename Match> u8 *LastMatch(const u8 *owner, Match match) {
    u8 *found = nullptr;
    for (u8 *node = First(owner); node; node = at<u8 *>(node, 4)) {
        u8 *item = at<u8 *>(node, 0);
        if (match(item))
            found = item;
    }
    return found;
}

}  // namespace

extern "C" {

u8 *__fastcall Owner2c_FindByKey(u8 *owner, void *, u32 key) {
    return LastMatch(owner, [&](const u8 *item) { return at<u32>(item, 8) == key; });
}

u8 *__fastcall Owner2c_At(u8 *owner, void *, s32 index) {
    u8 *found = nullptr;
    s32 i = 0;
    for (u8 *node = First(owner); node; node = at<u8 *>(node, 4), i++) {
        if (i == index)
            found = at<u8 *>(node, 0);
    }
    return found;
}

u32 __fastcall Owner2c_ValueByKey(u8 *owner, void *, u32 key) {
    u32 value = 0;
    for (u8 *node = First(owner); node; node = at<u8 *>(node, 4)) {
        const u8 *item = at<u8 *>(node, 0);
        if (at<u32>(item, 8) == key)
            value = at<u32>(item, 0x14);
    }
    return value;
}

u8 *__fastcall Owner2c_FindKind1(u8 *owner) {
    return LastMatch(owner, [](const u8 *item) { return at<u32>(item, 0xc) == 1; });
}

u8 *__fastcall Owner2c_FindKind100(u8 *owner) {
    return LastMatch(owner, [](const u8 *item) { return at<u32>(item, 0xc) == 0x64; });
}

u8 *__fastcall Owner2c_FindByPair(u8 *owner, void *, u32 a, u32 b) {
    return LastMatch(owner, [&](const u8 *item) { return at<u32>(item, 0x44) == a && at<u32>(item, 0x64) == b; });
}
}
