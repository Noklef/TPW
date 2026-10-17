#include "small51.h"

#include <cstdint>

#include "gameheap.h"
#include "small6.h"
#include "../world/objectmsg.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u8 *__fastcall Class703088_ScalarDtor(u8 *object, void *, u32 flags) {
    at<u32>(object, 0) = 0x00703088;
    u8 *const list = global<u8 *>(0x00fa6e38);
    u8 *node = at<u8 *>(list, 4);
    while (node && at<u8 *>(node, 0) != object)
        node = at<u8 *>(node, 4);
    if (node) {
        if (node == at<u8 *>(list, 4))
            at<u8 *>(list, 4) = at<u8 *>(node, 4);
        else
            at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4);
        if (node == at<u8 *>(list, 8))
            at<u8 *>(list, 8) = at<u8 *>(node, 8);
        else
            at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8);
        at<u32>(list, 0xc) = at<u32>(list, 0xc) - 1;
        GameHeap_Free(node);
    }
    PtrArray_RemoveAndFree(reinterpret_cast<u8 *>(0x00879ce0), nullptr, at<u32>(object, 0x28));
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

u32 __fastcall Object_QueueHasRoom(u8 *object) {
    const u32 position = (u32)Object_QueuePosition(object, nullptr, 0);
    return position < (at<u32>(object, 0x40) << 2) ? 1 : 0;
}

}  // extern "C"
