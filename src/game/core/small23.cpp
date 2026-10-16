#include "small23.h"

#include <cstdint>

#include "crt.h"
#include "gameheap.h"
#include "small20.h"
#include "small21.h"
#include "string.h"
#include "vobj3.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void SharedRelease(u8 *object, u32 elementSize, u32 destructorAt) {
    u8 *shared = at<u8 *>(object, 0x30);
    if (!shared)
        return;
    const u32 uses = at<u32>(shared, 0xc) - 1;
    at<u32>(shared, 0xc) = uses;
    if (uses == 0) {
        if (u8 *array = at<u8 *>(shared, 4)) {
            CRT_VectorDtorIterator(array, elementSize, at<s32>(array, (u32)-4), global<void(__fastcall *)(void *)>(destructorAt));
            GameHeap_Free(array - 4);
        }
        at<u32>(shared, 4) = 0;
        GameHeap_Free(shared);
    }
    at<u32>(object, 0x30) = 0;
}

// The lower bound in a tree whose nodes {left, parent, right, key} end at the sentinel: the last node passed on the way down whose key is not below the key.
template <typename K> u8 *LowerBound(u8 *tree, K key, u32 sentinelAt) {
    u8 *head = at<u8 *>(tree, 4);
    u8 *result = head;
    u8 *sentinel = global<u8 *>(sentinelAt);
    u8 *node = at<u8 *>(head, 4);
    while (node != sentinel) {
        if (at<K>(node, 0xc) < key) {
            node = at<u8 *>(node, 8);
        } else {
            result = node;
            node = at<u8 *>(node, 0);
        }
    }
    return result;
}

}  // namespace

extern "C" {

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

u32 __fastcall Names_FindLastValue(u8 *object, void *, const char *name) {
    u32 value = 1;
    u8 *node = at<u8 *>(at<u8 *>(object, 0x2c), 4);
    while (node) {
        u8 *next = at<u8 *>(node, 4);
        u8 *entry = at<u8 *>(node, 0);
        if (Crt().stricmp(at<const char *>(entry, 0x14), name) == 0)
            value = at<u32>(entry, 8);
        node = next;
    }
    return value;
}

void __fastcall Shared_Release66(u8 *object) { SharedRelease(object, 0x66, 0x006fd504); }

void __fastcall Shared_Release10(u8 *object) { SharedRelease(object, 0x10, 0x006fd50c); }

u8 *__fastcall Tree16a_LowerBound(u8 *tree, void *, const u16 *key) { return LowerBound<u16>(tree, *key, 0x00785de4); }

u8 *__fastcall Tree32_LowerBound(u8 *tree, void *, const u32 *key) { return LowerBound<u32>(tree, *key, 0x007b4a54); }

u8 *__fastcall Tree16b_LowerBound(u8 *tree, void *, const u16 *key) { return LowerBound<u16>(tree, *key, 0x00f7c970); }

u32 __cdecl Global879a24_Free() {
    u8 *object = global<u8 *>(0x00879a24);
    if (!object)
        return 0;
    Obj_ReleaseSevenComMembers(object);
    object = global<u8 *>(0x00879a24);
    if (object) {
        Obj_ReleaseSevenComMembers(object);
        GameHeap_Free(object);
    }
    global<u32>(0x00879a24) = 0;
    return 1;
}

u32 __fastcall Obj_ReleaseSevenIfMode1(u8 *object) {
    if (global<u32>(0x008bd524) == 1)
        return Obj_ReleaseSevenComMembers(object);
    return 1;
}

void __cdecl Globals816d40_Free() {
    if (!global<u32>(0x00816d40))
        return;
    global<u32>(0x0080ced0) = 0;
    Mem_FreeChecked(global<void *>(0x0080cef8));
    const u32 other = global<u32>(0x00816d38);
    global<u32>(0x00816d40) = 0;
    Mem_FreeChecked((void *)(uintptr_t)other);
}

u8 *__fastcall Strings5_Ctor(u8 *object) {
    for (u32 offset = 0; offset <= 0x50; offset += 0x14)
        String_Ctor((String *)(object + offset));
    return object;
}

}  // extern "C"
