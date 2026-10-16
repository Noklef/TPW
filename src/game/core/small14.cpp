#include "small14.h"

#include <cstdint>

#include "gameheap.h"
#include "small8.h"
#include "small9.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// One use less for a counted buffer whose count is the block's first byte, or the block freed.
void ReleaseCounted1(u8 *buffer) {
    const u8 uses = buffer[-1];
    if (uses == 0 || uses == 0xff)
        GameHeap_Free(buffer - 1);
    else
        buffer[-1] = (u8)(uses - 1);
}

}  // namespace

extern "C" {

u8 *__fastcall CountedVector14_Deleter(u8 *object, void *, u32 flags) {
    CountedVector14_Clear(object);
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

u32 __fastcall List_PushHeadForward(u8 *list, void *, u32 item) { return List_PushHead(list, nullptr, item); }

void __stdcall CountedVector14_ClearRange(u8 *first, u8 *last) {
    for (u8 *element = first; element != last; element += 0x24)
        CountedVector14_Clear(element);
}

void __fastcall CountedList_ClearRelease(u8 *object) {
    u8 *sentinel = at<u8 *>(object, 4);
    u8 *node = at<u8 *>(sentinel, 0);
    while (node != sentinel) {
        u8 *erased = node;
        node = at<u8 *>(node, 0);
        at<u32>(at<u8 *>(erased, 4), 0) = at<u32>(erased, 0);
        at<u32>(at<u8 *>(erased, 0), 4) = at<u32>(erased, 4);
        CountedBuf_ReleaseAndClear(erased + 8);
        GameHeap_Free(erased);
        at<u32>(object, 8) -= 1;
    }
    GameHeap_Free(at<void *>(object, 4));
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
}

void __fastcall Counted2_Clear(u8 *object) {
    if (at<void *>(object, 0x20)) {
        GameHeap_Free(at<void *>(object, 0x20));
        at<u32>(object, 0x20) = 0;
    }
    if (at<u8 *>(object, 0x14))
        ReleaseCounted1(at<u8 *>(object, 0x14));
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x1c) = 0;
    if (at<u8 *>(object, 4))
        ReleaseCounted1(at<u8 *>(object, 4));
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
}

}  // extern "C"
