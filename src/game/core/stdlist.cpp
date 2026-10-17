#include "stdlist.h"

#include <cstdint>
#include <cstring>

#include "accessors.h"
#include "buffers.h"
#include "crt.h"
#include "gameheap.h"
#include "string.h"

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

void EraseMatching(u8 *object, u32 sentinelOffset, u32 countOffset) {
    u8 *sentinel = at<u8 *>(object, sentinelOffset);
    u8 *key = at<u8 *>(object, 0x7c) + 8;
    u8 *node = at<u8 *>(sentinel, 0);
    while (node != sentinel) {
        if (at<u32>(node, 8) == at<u32>(key, 0)) {
            u8 *erased = node;
            node = at<u8 *>(node, 0);
            at<u32>(at<u8 *>(erased, 4), 0) = at<u32>(erased, 0);
            at<u32>(at<u8 *>(erased, 0), 4) = at<u32>(erased, 4);
            GameHeap_Free(erased);
            at<u32>(object, countOffset) -= 1;
        } else {
            node = at<u8 *>(node, 0);
        }
    }
}

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

void __fastcall List68_EraseMatching(u8 *object) { EraseMatching(object, 0x68, 0x6c); }

void __fastcall List74_EraseMatching(u8 *object) { EraseMatching(object, 0x74, 0x78); }

u32 *__fastcall List_EraseNodeCounted(u8 *object, void *, u32 *out, u8 *node) {
    u8 *next = at<u8 *>(node, 0);
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
    at<u32>(object, 8) -= 1;
    *out = (u32)(uintptr_t)next;
    return out;
}

u32 *__fastcall List_EraseRangeCounted(u8 *object, void *, u32 *out, u8 *first, u8 *last) {
    while (first != last) {
        u8 *node = first;
        first = at<u8 *>(node, 0);
        at<u32>(at<u8 *>(node, 4), 0) = at<u32>(node, 0);
        at<u32>(at<u8 *>(node, 0), 4) = at<u32>(node, 4);
        u8 *buffer = at<u8 *>(node, 0xc);
        if (buffer) {
            const u8 uses = buffer[-1];
            if (uses == 0 || uses == 0xff)
                GameHeap_Free(buffer - 2);
            else
                buffer[-1] = (u8)(uses - 1);
        }
        at<u32>(node, 0xc) = 0;
        at<u32>(node, 0x10) = 0;
        at<u32>(node, 0x14) = 0;
        GameHeap_Free(node);
        at<u32>(object, 8) -= 1;
    }
    *out = (u32)(uintptr_t)first;
    return out;
}

u32 *__fastcall List_EraseNodeCounted2(u8 *object, void *, u32 *out, u8 *node) {
    u8 *next = at<u8 *>(node, 0);
    at<u32>(at<u8 *>(node, 4), 0) = at<u32>(node, 0);
    at<u32>(at<u8 *>(node, 0), 4) = at<u32>(node, 4);
    u8 *buffer = at<u8 *>(node, 0xc);
    if (buffer) {
        const u8 uses = buffer[-1];
        if (uses == 0 || uses == 0xff)
            GameHeap_Free(buffer - 2);
        else
            buffer[-1] = (u8)(uses - 1);
    }
    at<u32>(node, 0xc) = 0;
    at<u32>(node, 0x10) = 0;
    at<u32>(node, 0x14) = 0;
    GameHeap_Free(node);
    at<u32>(object, 8) -= 1;
    *out = (u32)(uintptr_t)next;
    return out;
}

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

u32 __fastcall List_PushHeadForward(u8 *list, void *, u32 item) { return List_PushHead(list, nullptr, item); }

u8 *__stdcall Node14_New(u32 a, u32 b) {
    u8 *node = (u8 *)GameHeap_Alloc(0x14);
    at<u32>(node, 4) = a;
    at<u32>(node, 0x10) = b;
    return node;
}

u8 *__fastcall Sentinel_Ctor(u8 *object, void *, const u8 *from) {
    at<u8>(object, 0) = at<u8>(from, 0);
    u8 *node = (u8 *)GameHeap_Alloc(0xc);
    at<u8 *>(node, 0) = node;
    at<u8 *>(node, 4) = node;
    at<u8 *>(object, 4) = node;
    at<u32>(object, 8) = 0;
    return object;
}

u8 *__fastcall List_PopHeadObject(u8 *list) {
    u8 *node = at<u8 *>(list, 4);
    if (!node)
        return nullptr;
    u8 *next = at<u8 *>(node, 4);
    at<u8 *>(list, 4) = next;
    if (next)
        at<u32>(next, 8) = 0;
    else
        at<u32>(list, 8) = 0;
    const u32 left = at<u32>(list, 0xc) - 1;
    u8 *held = at<u8 *>(node, 0);
    at<u32>(list, 0xc) = left;
    GameHeap_Free(node);
    return held;
}

void __fastcall List_SpliceOne(u8 *list, void *, u8 *a, u8 *x, u8 *c) {
    if (a == c)
        return;
    u8 *n = at<u8 *>(c, 0);
    if (a == n)
        return;
    at<u8 *>(at<u8 *>(n, 4), 0) = a;
    at<u8 *>(at<u8 *>(c, 4), 0) = n;
    at<u8 *>(at<u8 *>(a, 4), 0) = c;
    u8 *nPrev = at<u8 *>(n, 4);
    u8 *aPrev = at<u8 *>(a, 4);
    at<u8 *>(a, 4) = nPrev;
    at<u8 *>(n, 4) = at<u8 *>(c, 4);
    at<u8 *>(c, 4) = aPrev;
    at<u32>(list, 8) += 1;
    at<u32>(x, 8) -= 1;
}

void __fastcall StdListString_Unique(u8 *list) {
    u8 *const head = at<u8 *>(list, 4);
    u8 *previous = at<u8 *>(head, 0);
    if (previous == head) return;
    u8 *current = at<u8 *>(previous, 0);
    if (current == head) return;
    do {
        const u8 *characters = at<const u8 *>(current, 0xc);
        const u32 length = at<u32>(current, 0x10);
        if (!characters) characters = reinterpret_cast<const u8 *>(0x006fe5b0);
        const u32 previousLength = at<u32>(previous, 0x10);
        const u8 *other = at<const u8 *>(previous, 0xc);
        bool equal = previousLength == length;
        if (equal) {
            for (u32 i = 0; i < length; ++i) {
                if (characters[i] != other[i]) { equal = false; break; }
            }
        }
        if (equal) {
            at<u8 *>(at<u8 *>(current, 4), 0) = at<u8 *>(current, 0);
            at<u8 *>(at<u8 *>(current, 0), 4) = at<u8 *>(current, 4);
            u8 *const text = at<u8 *>(current, 0xc);
            if (text) {
                const u8 count = text[-1];
                if (count == 0 || count == 0xff) GameHeap_Free(text - 1);
                else text[-1] = (u8)(count - 1);
            }
            at<u32>(current, 0xc) = 0;
            at<u32>(current, 0x10) = 0;
            at<u32>(current, 0x14) = 0;
            GameHeap_Free(current);
            at<u32>(list, 8) = at<u32>(list, 8) - 1;
        } else {
            previous = current;
        }
        current = at<u8 *>(previous, 0);
    } while (current != head);
}

void __fastcall StdListDword_Merge(u8 *list, void *, u8 *other) {
    if (other == list) return;
    u8 *const head1 = at<u8 *>(list, 4);
    u8 *first1 = at<u8 *>(head1, 0);
    u8 *const head2 = at<u8 *>(other, 4);
    u8 *first2 = at<u8 *>(head2, 0);
    while (first1 != head1 && first2 != head2) {
        if (at<u32>(first2, 8) < at<u32>(first1, 8)) {
            u8 *const next2 = at<u8 *>(first2, 0);
            at<u8 *>(at<u8 *>(next2, 4), 0) = first1;
            at<u8 *>(at<u8 *>(first2, 4), 0) = next2;
            at<u8 *>(at<u8 *>(first1, 4), 0) = first2;
            u8 *const saved = at<u8 *>(first1, 4);
            at<u8 *>(first1, 4) = at<u8 *>(next2, 4);
            at<u8 *>(next2, 4) = at<u8 *>(first2, 4);
            at<u8 *>(first2, 4) = saved;
            first2 = next2;
        } else {
            first1 = at<u8 *>(first1, 0);
        }
    }
    if (first2 != head2) {
        at<u8 *>(at<u8 *>(head2, 4), 0) = head1;
        at<u8 *>(at<u8 *>(first2, 4), 0) = head2;
        at<u8 *>(at<u8 *>(head1, 4), 0) = first2;
        u8 *const saved = at<u8 *>(head1, 4);
        at<u8 *>(head1, 4) = at<u8 *>(head2, 4);
        at<u8 *>(head2, 4) = at<u8 *>(first2, 4);
        at<u8 *>(first2, 4) = saved;
    }
    at<u32>(list, 8) = at<u32>(list, 8) + at<u32>(other, 8);
    at<u32>(other, 8) = 0;
}

u8 *__stdcall Node28_New(u32 a, u32 b) {
    u8 *node = static_cast<u8 *>(GameHeap_Alloc(0x28));
    at<u32>(node, 4) = a;
    at<u32>(node, 0x24) = b;
    return node;
}

u8 *__stdcall Node18_New(u32 a, u32 b) {
    u8 *node = static_cast<u8 *>(GameHeap_Alloc(0x18));
    at<u32>(node, 0) = a ? a : (u32)(uintptr_t)node;
    at<u32>(node, 4) = b ? b : (u32)(uintptr_t)node;
    return node;
}

u8 *__fastcall ListHead_Ctor(u8 *object) {
    u8 top = (u8)((u32)(uintptr_t)object >> 24);  // sic
    at<u8>(object, 0) = top;
    u8 *head = static_cast<u8 *>(GameHeap_Alloc(0x18));
    at<u8 *>(head, 0) = head;
    at<u8 *>(head, 4) = head;
    at<u8 *>(object, 4) = head;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = at<u32>(head, 0);
    return object;
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

u8 *__fastcall ListHead_CtorFrom(u8 *object, void *, const u8 *source) {
    at<u8>(object, 0) = at<u8>(source, 0);
    u8 *head = static_cast<u8 *>(GameHeap_Alloc(0x18));
    at<u8 *>(head, 0) = head;
    at<u8 *>(head, 4) = head;
    at<u8 *>(object, 4) = head;
    at<u32>(object, 8) = 0;
    return object;
}

u8 *__stdcall Node0c_New(u32 a, u32 b) {
    u8 *node = static_cast<u8 *>(GameHeap_Alloc(0xc));
    at<u32>(node, 0) = a ? a : (u32)(uintptr_t)node;
    at<u32>(node, 4) = b ? b : (u32)(uintptr_t)node;
    return node;
}

u32 __fastcall List_PushHead(u8 *list, void *, u32 item) {
    u8 *node = static_cast<u8 *>(GameHeap_Alloc(0xc));
    if (node) {
        at<u32>(node, 4) = 0;
        at<u32>(node, 0) = item;
        at<u32>(node, 8) = 0;
    }
    at<u32>(node, 4) = at<u32>(list, 4);
    u8 *head = at<u8 *>(list, 4);
    if (!head) {
        at<u8 *>(list, 8) = node;
        at<u8 *>(list, 4) = node;
        at<u32>(list, 0xc) += 1;
        return (u32)(uintptr_t)node;
    }
    at<u8 *>(head, 8) = node;
    at<u8 *>(list, 4) = node;
    const u32 count = at<u32>(list, 0xc) + 1;
    at<u32>(list, 0xc) = count;
    return count;
}

void __fastcall CountedListPair_Dtor(u8 *pair) {
    u8 *second = pair + 0xc;
    u8 *head = at<u8 *>(pair, 0x10);
    u32 out;
    for (u8 *node = at<u8 *>(head, 0); node != head;) {
        u8 *const erase = node;
        node = at<u8 *>(node, 0);
        List_EraseNodeCounted2(second, nullptr, &out, erase);
    }
    GameHeap_Free(at<void *>(second, 4));
    at<u32>(second, 4) = 0;
    at<u32>(second, 8) = 0;
    head = at<u8 *>(pair, 4);
    for (u8 *node = at<u8 *>(head, 0); node != head;) {
        u8 *const erase = node;
        node = at<u8 *>(node, 0);
        at<u32>(at<u8 *>(erase, 4), 0) = at<u32>(erase, 0);
        at<u32>(at<u8 *>(erase, 0), 4) = at<u32>(erase, 4);
        CountedBuf_Deleter(erase + 8, nullptr, 0);
        GameHeap_Free(erase);
        at<u32>(pair, 8) -= 1;
    }
    GameHeap_Free(at<void *>(pair, 4));
    at<u32>(pair, 4) = 0;
    at<u32>(pair, 8) = 0;
}
}  // extern "C"
