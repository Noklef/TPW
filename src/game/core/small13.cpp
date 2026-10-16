#include "small13.h"

#include <cstdint>

#include "gameheap.h"
#include "small12.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// Unlinks the node and releases its counted buffer (the count is the block's first byte), clears the fields and frees the node.
void EraseCounted1(u8 *node) {
    at<u32>(at<u8 *>(node, 4), 0) = at<u32>(node, 0);
    at<u32>(at<u8 *>(node, 0), 4) = at<u32>(node, 4);
    u8 *buffer = at<u8 *>(node, 0xc);
    if (buffer) {
        const u8 uses = buffer[-1];
        if (uses == 0 || uses == 0xff)
            GameHeap_Free(buffer - 1);
        else
            buffer[-1] = (u8)(uses - 1);
    }
    at<u32>(node, 0xc) = 0;
    at<u32>(node, 0x10) = 0;
    at<u32>(node, 0x14) = 0;
    GameHeap_Free(node);
}

}  // namespace

extern "C" {

u32 *__fastcall List_EraseRangeCounted1(u8 *object, void *, u32 *out, u8 *first, u8 *last) {
    while (first != last) {
        u8 *node = first;
        first = at<u8 *>(node, 0);
        EraseCounted1(node);
        at<u32>(object, 8) -= 1;
    }
    *out = (u32)(uintptr_t)first;
    return out;
}

u8 __fastcall Table76e300_LimitCheck(const u8 *object, void *, u32 id, s32 limit) {
    const u32 record = Table76e300_FindRecord(id);
    const u32 index = record ? global<u32>(record + 8) : 10;
    if (index == 10)
        return 0;
    const s32 value = at<s32>(object, 0x2c + 12 * index);
    if (value == 0)
        return 0;
    return limit >= value;
}

u32 __fastcall Obj_HasActivity(const u8 *object) {
    if (at<u32>(object, 0) || at<u32>(object, 0x6c))
        return 1;
    for (u32 offset = 0xe0; offset <= 0x100; offset += 4) {
        if (at<u32>(at<u8 *>(object, offset), 0xc))
            return 1;
    }
    if (at<u32>(object, 0x324) || at<u32>(object, 0x38c))
        return 1;
    return 0;
}

u8 __fastcall List_EraseNthCounted(u8 *object, void *, u32 index) {
    u8 *sentinel = at<u8 *>(object, 4);
    u8 *node = at<u8 *>(sentinel, 0);
    u32 i = 0;
    while (node != sentinel) {
        if (i++ == index) {
            EraseCounted1(node);
            at<u32>(object, 8) -= 1;
            return 1;
        }
        node = at<u8 *>(node, 0);
    }
    return 0;
}

}  // extern "C"
