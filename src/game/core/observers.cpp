#include "observers.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

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

u32 __fastcall Obj_SetAt8AndAdd(u8 *object, void *, u8 *item) {
    at<u8 *>(object, 8) = item;
    return ObserverList0_Add(object, nullptr, item);
}
}  // extern "C"
