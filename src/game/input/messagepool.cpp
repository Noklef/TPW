// The input messages' pools: see messagepool.h.
#include "messagepool.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "devicefilter.h"

namespace {

struct Node {
    Node *next, *prev;
    u8 *message;
};

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

// A mouse message's fields from a record (type, value, time).
void fillMouse(u8 *message, u32 code, const u32 *record) {
    at<u32>(message, 0x10) = code;
    at<u32>(message, 0x20) = record[1] >> 4 & 8;
    at<u32>(message, 0x14) = 0;
    at<u32>(message, 0x18) = 0;
    at<u8>(message, 0x1c) = static_cast<u8>(record[1]);
    at<u32>(message, 4) = record[2];
    at<u32>(message, 8) = 1;
}

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

void addLast(u8 *self, u8 *message) {
    Node *const sentinel = at<Node *>(self, 8);
    Node *const last = sentinel->prev;
    Node *const node = static_cast<Node *>(GameHeap_Alloc(0xc));
    node->next = sentinel ? sentinel : node;
    node->prev = last ? last : node;
    sentinel->prev = node;
    node->prev->next = node;
    node->message = message;
    ++at<u32>(self, 0xc);
}

u32 next(u8 *self, u8 *(*make)()) {
    Node *const start = at<Node *>(self, 0x10);
    while (at<u32>(at<Node *>(self, 0x10)->message, 8)) {
        at<Node *>(self, 0x10) = at<Node *>(self, 0x10)->next;
        if (at<Node *>(self, 0x10) == at<Node *>(self, 8)) at<Node *>(self, 0x10) = at<Node *>(self, 8)->next;
        if (at<Node *>(self, 0x10) != start) continue;
        u8 *const message = make();
        addLast(self, message);
        at<Node *>(self, 0x10) = at<Node *>(self, 8)->prev;
    }
    return static_cast<u32>(reinterpret_cast<uintptr_t>(at<Node *>(self, 0x10)->message));
}

// A pool's constructor, as the sources' constructors have it inline: the empty list, one message, the cursor on it. (The list's allocator byte at +4 is stack garbage in the
// originals, which nothing reads: not written.)
void initPool(u8 *self, u32 vtable, u8 *(*make)()) {
    Node *const sentinel = static_cast<Node *>(GameHeap_Alloc(0xc));
    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    at<Node *>(self, 8) = sentinel;
    at<u32>(self, 0xc) = 0;
    at<u32>(self, 0) = vtable;
    addLast(self, make());
    at<Node *>(self, 0x10) = at<Node *>(self, 8)->next;
}

}  // namespace

extern "C" {

void __fastcall KeyPool_Dtor(u8 *self) { destroy(self, 0x00703d1c); }
void __fastcall MousePool_Dtor(u8 *self) { destroy(self, 0x00703d14); }
u8 *__fastcall KeyPool_DeletingDtor(u8 *self, void *, u32 flags) {
    KeyPool_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u8 *__fastcall MousePool_DeletingDtor(u8 *self, void *, u32 flags) {
    MousePool_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u8 *__fastcall MessageSource_Ctor(u8 *self) {
    DeviceFilter_Ctor(self);
    initPool(self + 0x34, 0x00703d1c, newKeyMessage);
    initPool(self + 0x48, 0x00703d14, newMouseMessage);
    at<u32>(self, 0) = 0x00703cf8;
    at<u32>(self, 0x10) = 0x00703ce8;
    return self;
}

void __fastcall MessageSource_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00703cf8;
    at<u32>(self, 0x10) = 0x00703ce8;
    MousePool_Dtor(self + 0x48);
    KeyPool_Dtor(self + 0x34);
    DeviceFilter_Dtor(self);
}

u8 *__fastcall MessageSource_DeletingDtor(u8 *self, void *, u32 flags) {
    MessageSource_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u8 *__fastcall KeySource_Ctor(u8 *self) {
    DeviceFilter_Ctor(self);
    initPool(self + 0x34, 0x00703d14, newMouseMessage);
    at<u32>(self, 0x18) = 6;
    at<u32>(self, 0x14) = 6;
    at<u32>(self, 0) = 0x00703d38;
    at<u32>(self, 0x10) = 0x00703d28;
    at<u32>(self, 0x34) = 0x00703d20;
    return self;
}

void __fastcall KeySource_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x00703d38;
    at<u32>(self, 0x10) = 0x00703d28;
    at<u32>(self, 0x34) = 0x00703d20;
    MousePool_Dtor(self + 0x34);
    DeviceFilter_Dtor(self);
}

u8 *__fastcall KeySource_DeletingDtor(u8 *self, void *, u32 flags) {
    KeySource_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

void __fastcall MessageSource_Deliver(u8 *self, void *, u32 count) {
    const u32 *record = at<const u32 *>(self, 0x30);
    for (; count; --count, record += 4) {
        const u32 type = record[0];
        if (type > 0xf) continue;
        if (type == 0 || type == 4 || type == 8) {
            u8 *const message = reinterpret_cast<u8 *>(static_cast<uintptr_t>(KeyPool_Next(self + 0x34)));
            at<u32>(message, 0x10) = record[1];
            at<u32>(message, 0xc) = type;
            at<u32>(message, 4) = record[2];
            at<u32>(message, 8) = 1;
            vcall<void>(at<u8 *>(self, 0xc), 0x14, message);
        } else if (type >= 0xc) {
            u8 *const message = reinterpret_cast<u8 *>(static_cast<uintptr_t>(MousePool_Next(self + 0x48)));
            fillMouse(message, 0xf0 + type - 0xc, record);
            vcall<void>(at<u8 *>(self, 0xc), 0x18, message);
        }
    }
}

void __fastcall MessageSource_DeliverKeys(u8 *self, void *, u32 count) {
    const u32 *record = at<const u32 *>(self, 0x30);
    for (; count; --count, record += 4) {
        u8 *const message = reinterpret_cast<u8 *>(static_cast<uintptr_t>(MousePool_Next(self + 0x34)));
        fillMouse(message, record[0] & 0xff, record);
        vcall<void>(at<u8 *>(self, 0xc), 0x18, message);
    }
}

u32 __fastcall KeyPool_Next(u8 *self) { return next(self, &newKeyMessage); }
u32 __fastcall MousePool_Next(u8 *self) { return next(self, &newMouseMessage); }

u32 *__fastcall MessageSource_Open(u8 *self, void *, u32 *result) {
    u32 local;
    DeviceFilter_Open(self, nullptr, &local);
    *result = local;
    return result;
}

}  // extern "C"
