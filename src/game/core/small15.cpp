#include "small15.h"

#include <cstdint>

#include "../world/objectdefs.h"
#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 ObserverList_Add(u8 *&listField, u32 vtable, u8 *item, u32 backOffset, u8 *owner) {
    if (!listField) {
        u8 *list = static_cast<u8 *>(GameHeap_Alloc(0x10));
        if (list) {
            at<u32>(list, 4) = 0;
            at<u32>(list, 8) = 0;
            at<u32>(list, 0xc) = 0;
            at<u32>(list, 0) = vtable;
        }
        listField = list;
    }
    u8 *list = listField;
    if (!list)
        return 0;
    u8 *node = static_cast<u8 *>(GameHeap_Alloc(0xc));
    if (node) {
        at<u8 *>(node, 0) = item;
        at<u32>(node, 4) = 0;
        at<u32>(node, 8) = 0;
    }
    at<u8 *>(node, 4) = at<u8 *>(list, 4);
    u8 *head = at<u8 *>(list, 4);
    if (!head)
        at<u8 *>(list, 8) = node;
    else
        at<u8 *>(head, 8) = node;
    at<u8 *>(list, 4) = node;
    at<u32>(list, 0xc) += 1;
    at<u8 *>(item, backOffset) = owner;
    return 1;
}

}  // namespace

extern "C" {

u32 __fastcall ObserverList4c_Add(u8 *object, void *, u8 *item) {
    return ObserverList_Add(at<u8 *>(object, 0x4c), 0x006fdb04, item, 0x34, object);
}

u32 __fastcall ObserverList0_Add(u8 *object, void *, u8 *item) {
    return ObserverList_Add(at<u8 *>(object, 0), 0x006fdb0c, item, 0x30, object);
}

void __fastcall Obj_ReplaceHandle24(u8 *object, void *, u32 handle) {
    u8 *replacement = global<u8 *>(0x007cfb90 + 20 * (handle & 0xffff));
    const u16 old = at<u16>(object, 0x24);
    if (old != 0)
        at<u16>(global<u8 *>(0x007cfb90 + 20 * (u32)old), 8) = (u16)handle;
    at<u16>(replacement, 8) = 0;
    at<u16>(replacement, 0xa) = at<u16>(object, 0x24);
    at<u16>(object, 0x24) = (u16)handle;
}

u32 __fastcall Obj_DefKind123(const u8 *object) {
    u8 *defs = reinterpret_cast<u8 *>(0x007890a0);
    if (at<u32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x9c) == 1)
        return 1;
    if (at<u32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x9c) == 2)
        return 1;
    return at<u32>(ObjectDefs_Find(defs, nullptr, at<u16>(object, 0xe)), 0x9c) == 3 ? 1 : 0;
}

void __fastcall Ring_PushUnique(u8 *object, void *, u32 id, u32 value) {
    const u32 position = at<u32>(object, 0x88);
    const u8 *last = position >= 1 ? object + position * 4 : object + position * 4 + 0x80;
    if (at<u16>(last, 0) == id && at<u16>(last, 2) == (u16)value)
        return;
    at<u16>(object, position * 4 + 4) = (u16)id;
    at<u16>(object, position * 4 + 6) = (u16)value;
    at<u32>(object, 0x88) = (at<u32>(object, 0x88) + 1) & 0x1f;
}

}  // extern "C"
