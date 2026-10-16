#include "small6.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void ReleaseCounted(u8 *buffer) {
    u8 uses = buffer[-1];
    if (uses == 0 || uses == 0xff)
        GameHeap_Free(buffer - 2);
    else
        buffer[-1] = (u8)(uses - 1);
}

void ClearCounted(u8 *object) {
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
}

}  // namespace

extern "C" {

void __stdcall CountedEntries_Release(u8 *first, u8 *last) {
    for (u8 *entry = first; entry != last; entry += 0x10) {
        if (at<u8 *>(entry, 4))
            ReleaseCounted(at<u8 *>(entry, 4));
        ClearCounted(entry);
    }
}

void __fastcall CountedBuf_Reset(u8 *object, void *, u32 release) {
    if ((u8)release && at<u8 *>(object, 4))
        ReleaseCounted(at<u8 *>(object, 4));
    ClearCounted(object);
}

u8 *__fastcall CountedBuf_Deleter(u8 *object, void *, u32 flags) {
    if (at<u8 *>(object, 4))
        ReleaseCounted(at<u8 *>(object, 4));
    ClearCounted(object);
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

u32 __fastcall PtrArray_RemoveAndFree(u8 *object, void *, u32 item) {
    s32 index = at<s32>(object, 0x1c) - 1;
    u32 *items = at<u32 *>(object, 8);
    while (index >= 0 && items[index] != item)
        --index;
    if (index < 0)
        return 0;
    GameHeap_Free(reinterpret_cast<void *>(item));
    s32 count = at<s32>(object, 0x1c) - 1;
    at<s32>(object, 0x1c) = count;
    items = at<u32 *>(object, 8);
    items[index] = items[count];
    at<s32>(at<u8 *>(object, 4), 4) = at<s32>(object, 0x1c);
    return 1;
}

u32 *__fastcall List_EraseRange(u8 *object, void *, u32 *out, u8 *first, u8 *last) {
    while (first != last) {
        u8 *node = first;
        first = at<u8 *>(node, 0);
        at<u32>(at<u8 *>(node, 4), 0) = at<u32>(node, 0);
        at<u32>(at<u8 *>(node, 0), 4) = at<u32>(node, 4);
        GameHeap_Free(node);
        at<s32>(object, 8) -= 1;
    }
    *out = (u32)(uintptr_t)first;
    return out;
}

}  // extern "C"
