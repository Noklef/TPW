#include "small48.h"

#include <cstdint>

#include "gameheap.h"
#include "small38.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 *__fastcall StdMapG_InsertCopy(u8 *tree, void *, u32 *out, const u16 *value) {
    u32 result[2];
    StdMapG_Insert(tree, nullptr, reinterpret_cast<u8 **>(result), value);
    at<u32>(out, 0) = result[0];
    at<u8>(out, 4) = (u8)result[1];
    return out;
}

u8 __fastcall Object_AddKeyed(u8 *object, void *, u32 key, s32 amount) {
    u32 result[2];
    u16 value = (u16)key;
    StdMapG_Insert(object + 0x28, nullptr, reinterpret_cast<u8 **>(result), &value);
    if ((u8)result[1] == 0) return 0;
    at<s32>(object, 0x1c) += amount;
    return 1;
}

void __fastcall Class7037b8_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x007037b8;
    u8 *const head = at<u8 *>(object, 8);
    for (u8 *node = at<u8 *>(head, 0); node != at<u8 *>(object, 8); node = at<u8 *>(node, 0))
        GameHeap_Free(at<void *>(node, 8));
    u8 *const list = at<u8 *>(object, 8);
    u8 *node = at<u8 *>(list, 0);
    while (node != list) {
        u8 *const next = at<u8 *>(node, 0);
        at<u8 *>(at<u8 *>(node, 4), 0) = at<u8 *>(node, 0);
        at<u8 *>(at<u8 *>(node, 0), 4) = at<u8 *>(node, 4);
        GameHeap_Free(node);
        at<u32>(object, 0xc) = at<u32>(object, 0xc) - 1;
        node = next;
    }
    GameHeap_Free(at<void *>(object, 8));
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
}

}  // extern "C"
