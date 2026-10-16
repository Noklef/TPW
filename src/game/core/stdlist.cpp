#include "stdlist.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// A new node of `size` bytes linked in front of `before` (its +0 the next, +4 the previous; a null `before` or previous node is taken to be the node itself).
u8 *LinkBefore(u8 *before, u32 size) {
    u8 *node = (u8 *)GameHeap_Alloc(size);
    u8 *prev = before ? at<u8 *>(before, 4) : nullptr;
    at<u8 *>(node, 0) = before ? before : node;
    at<u8 *>(node, 4) = prev ? prev : node;
    at<u8 *>(before, 4) = node;  // (a null `before` faults, as in the original)
    at<u8 *>(at<u8 *>(node, 4), 0) = node;
    return node;
}

void CopyString(u8 *node, const StdString *x) {
    StdString *s = (StdString *)(node + 8);
    *(u8 *)s = *(const u8 *)x;  // (only the first byte of the empty allocator is copied)
    s->ptr = nullptr;
    s->len = 0;
    s->res = 0;
    StdString_AssignSub(s, nullptr, x, 0, global<u32>(0x006fe5ac));
}

}  // namespace

extern "C" {

void __fastcall StdListString_Clear(u8 *list) {
    u8 *sentinel = at<u8 *>(list, 4);
    for (u8 *node = at<u8 *>(sentinel, 0); node != sentinel;) {
        u8 *next = at<u8 *>(node, 0);
        at<u8 *>(at<u8 *>(node, 4), 0) = at<u8 *>(node, 0);
        at<u8 *>(at<u8 *>(node, 0), 4) = at<u8 *>(node, 4);
        StdString_Tidy((StdString *)(node + 8), nullptr, 1);
        GameHeap_Free(node);
        at<u32>(list, 8) -= 1;
        node = next;
    }
    GameHeap_Free(at<u8 *>(list, 4));
    at<u32>(list, 4) = 0;
    at<u32>(list, 8) = 0;
}

void __fastcall StdListString_PushBack(u8 *list, void *, const StdString *x) {
    u8 *node = LinkBefore(at<u8 *>(list, 4), 0x18);
    CopyString(node, x);
    at<u32>(list, 8) += 1;
}

void __fastcall StdListString_Insert(u8 *list, void *, u8 **out, u8 *before, const StdString *x) {
    u8 *node = LinkBefore(before, 0x18);
    CopyString(node, x);
    at<u32>(list, 8) += 1;
    *out = node;
}

void __fastcall StdListDword_PushBack(u8 *list, void *, const u32 *value) {
    u8 *node = LinkBefore(at<u8 *>(list, 4), 0xc);
    at<u32>(node, 8) = *value;
    at<u32>(list, 8) += 1;
}

}  // extern "C"
