// The input messages' pools: see messagepool.h.
#include "messagepool.h"

#include <cstdint>

#include "../core/gameheap.h"

namespace {

struct Node {
    Node *next, *prev;
    u8 *message;
};

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

void destroy(u8 *self, u32 vtable) {
    at<u32>(self, 0) = vtable;
    for (Node *node = at<Node *>(self, 8)->next; node != at<Node *>(self, 8); node = node->next) GameHeap_Free(node->message);
    Node *const sentinel = at<Node *>(self, 8);
    for (Node *node = sentinel->next; node != sentinel;) {
        Node *const gone = node;
        node = node->next;
        gone->prev->next = gone->next;
        gone->next->prev = gone->prev;
        GameHeap_Free(gone);
        --at<u32>(self, 0xc);
    }
    GameHeap_Free(at<Node *>(self, 8));
    at<u32>(self, 8) = 0;
    at<u32>(self, 0xc) = 0;
}

u8 *newKeyMessage() {
    u8 *const message = static_cast<u8 *>(GameHeap_Alloc(0x14));
    if (message) {
        at<u32>(message, 8) = 0;
        at<u32>(message, 0) = 0x00703d18;
    }
    return message;
}

u8 *newMouseMessage() {
    u8 *const message = static_cast<u8 *>(GameHeap_Alloc(0x24));
    if (message) {
        at<u32>(message, 8) = 0;
        at<u32>(message, 0x14) = 0;
        at<u32>(message, 0x18) = 0;
        at<u32>(message, 0) = 0x00703d10;
    }
    return message;
}

u32 next(u8 *self, u8 *(*make)()) {
    Node *const start = at<Node *>(self, 0x10);
    while (at<u32>(at<Node *>(self, 0x10)->message, 8)) {
        at<Node *>(self, 0x10) = at<Node *>(self, 0x10)->next;
        if (at<Node *>(self, 0x10) == at<Node *>(self, 8)) at<Node *>(self, 0x10) = at<Node *>(self, 8)->next;
        if (at<Node *>(self, 0x10) != start) continue;
        u8 *const message = make();
        Node *const sentinel = at<Node *>(self, 8);
        Node *const last = sentinel->prev;
        Node *const node = static_cast<Node *>(GameHeap_Alloc(0xc));
        node->next = sentinel ? sentinel : node;
        node->prev = last ? last : node;
        sentinel->prev = node;
        node->prev->next = node;
        node->message = message;
        ++at<u32>(self, 0xc);
        at<Node *>(self, 0x10) = at<Node *>(self, 8)->prev;
    }
    return static_cast<u32>(reinterpret_cast<uintptr_t>(at<Node *>(self, 0x10)->message));
}

}  // namespace

extern "C" {

void __fastcall KeyPool_Dtor(u8 *self) { destroy(self, 0x00703d1c); }
void __fastcall MousePool_Dtor(u8 *self) { destroy(self, 0x00703d14); }
u32 __fastcall KeyPool_Next(u8 *self) { return next(self, &newKeyMessage); }
u32 __fastcall MousePool_Next(u8 *self) { return next(self, &newMouseMessage); }

}  // extern "C"
