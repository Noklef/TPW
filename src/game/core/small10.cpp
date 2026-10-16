#include "small10.h"

#include <cstdint>
#include <cstring>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *Class7028_Ctor(u8 *object, u32 vtable, u32 memberVtable, u32 finalVtable) {
    at<u32>(object, 4) = 0;
    at<u32>(object, 0x10) = 0;
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x1c) = 0;
    at<u32>(object, 0x20) = 0;
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x28) = 0;
    at<u32>(object, 0x30) = 0;
    at<u32>(object, 0) = vtable;
    at<u32>(object, 0x18) = 0;
    u8 *member = static_cast<u8 *>(GameHeap_Alloc(0x10));
    if (member) {
        at<u32>(member, 4) = 0;
        at<u32>(member, 8) = 0;
        at<u32>(member, 0xc) = 0;
        at<u32>(member, 0) = memberVtable;
        at<u8 *>(object, 0x2c) = member;
        at<u32>(object, 0) = finalVtable;
        return object;
    }
    at<u32>(object, 0) = finalVtable;
    at<u32>(object, 0x2c) = 0;
    return object;
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

}  // namespace

extern "C" {

u16 *__fastcall Ring16_Get(u8 *object, void *, u16 *out, s32 which, s32 index) {
    const u8 *record = object + 20 * (u32)(which + 0x18d2);
    const s32 capacity = at<s32>(record, 0xc);
    bool found = false;
    u16 value = 0;
    if (index < capacity) {
        const s32 head = at<s32>(record, 8);
        if (index <= head) {
            found = true;
            value = at<u16>(record, 2 * (u32)(head - index) + 2);
        } else if (at<u8>(record, 0x10)) {
            found = true;
            value = at<u16>(record, 2 * (u32)(head - index + capacity) + 2);
        }
    }
    *out = found ? value : 0;
    return out;
}

u8 __stdcall Table76e300_GetByte(u32 id) {
    if (id == 0x160 || id == 0x15f)
        return 0;
    const u32 slot = 0x0076e300 + 56 * id;
    if (global<u32>(slot + 4) == id)
        return global<u8>(slot + 0x20);
    for (u32 p = 0x0076e304; (s32)p < 0x00772fcc; p += 0x38) {
        if (global<u32>(p) == id)
            return global<u8>(slot + 0x20);
    }
    return 0;
}

u32 __fastcall Kinds_Compatible(const u8 *object, void *, u32 kind) {
    if (kind == 0)
        return 1;
    const u32 mine = at<u32>(object, 8);
    if (kind == 4 && mine == 4)
        return 0;
    if (kind == 0x15 && mine == 0x15)
        return 1;
    if (kind == 1 && mine == 3)
        return 1;
    if (kind == mine || mine == 0)
        return 1;
    if (kind == 4 && mine == 1)
        return 1;
    if (kind == 3 && mine == 1)
        return global<u32>(0x00820ab8) == 0 ? 1 : 0;
    return 0;
}

u8 *__fastcall Class7028b8_Ctor(u8 *object) { return Class7028_Ctor(object, 0x007028d0, 0x007028c8, 0x007028b8); }

u8 *__fastcall Class7028e0_Ctor(u8 *object) { return Class7028_Ctor(object, 0x007028f8, 0x007028f0, 0x007028e0); }

u8 *__fastcall Class702908_Ctor(u8 *object) { return Class7028_Ctor(object, 0x00702920, 0x00702918, 0x00702908); }

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

u8 __fastcall Array3D_Realloc(u8 *object, void *, u32 a, u32 b, u32 c) {
    if (at<void *>(object, 0xc))
        GameHeap_Free(at<void *>(object, 0xc));
    const u32 size = c * b * a;
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(size));
    at<u8 *>(object, 0xc) = block;
    if (!block)
        return 0;
    std::memset(block, 0, size);
    at<u32>(object, 8) = c;
    at<u32>(object, 0) = a;
    at<u32>(object, 4) = b;
    return 1;
}

void __cdecl KeyRecord_Push(u32 index, u32 shift, u32 a, u32 b, u32 c) {
    const u32 record = 0x00f86918 + 68 * index;
    u32 second;
    if (shift) {
        global<u32>(record + 4) = a;
        second = b;
    } else {
        global<u32>(record + 4) = global<u32>(record + 0xc);
        second = global<u32>(record + 0x10);
    }
    global<u32>(record + 8) = second;
    global<u32>(record + 0xc) = a;
    global<u32>(record + 0x10) = b;
    global<u32>(record + 0) = c;
}

}  // extern "C"
