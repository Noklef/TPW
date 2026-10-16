#include "small12.h"

#include <cstdint>

#include "gameheap.h"
#include "small6.h"
#include "small9.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

// The record of the table at 0x76e300 (56 bytes each) the lookups use: the one at the id's own slot when the id is found there or
// anywhere in the table (sic), otherwise 0; also 0 for the ids 0x15f and 0x160.
extern "C" u32 Table76e300_FindRecord(u32 id) {
    if (id == 0x160 || id == 0x15f)
        return 0;
    const u32 slot = 0x0076e300 + 56 * id;
    if (global<u32>(slot + 4) == id)
        return slot;
    for (u32 p = 0x0076e304; (s32)p < 0x00772fcc; p += 0x38) {
        if (global<u32>(p) == id)
            return slot;
    }
    return 0;
}

extern "C" {

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

u32 __fastcall Person_HasWordInState(const u8 *object, void *, u32 word) {
    if (!Person_HasWord(object, nullptr, word))
        return 0;
    const u8 kind = at<u8>(object, 2);
    if (kind == 5 && at<u32>(object, 0x19c) == 0xb)
        return 1;
    if (kind == 4 && at<u32>(object, 0x19c) == 0xd)
        return 1;
    return 0;
}

void __fastcall Setting_SetMode(u8 *object, void *, u32 mode) {
    at<u32>(object, 0) = mode;
    u32 &flags = global<u32>(0x007a1a8c);
    if (mode == 0)
        flags = (flags & 0xfcffffff) | 0x800000;
    else if (mode == 1)
        flags = (flags & 0xfd7fffff) | 0x1000000;
    else if (mode == 2)
        flags = (flags & 0xfe7fffff) | 0x2000000;
}

u32 __stdcall Quad_Push(const u32 *source, u32, u32, u8 *object) {
    const s32 count = at<s32>(object, 0x788);
    if (count < 4) {
        u8 *slot = object + 0x448 + 16 * (u32)count;
        at<s32>(object, 0x788) = count + 1;
        if (!source) {
            at<u32>(slot, 0) = 0;
            at<u32>(slot, 4) = 0;
            at<u32>(slot, 8) = 0;
            at<u32>(slot, 0xc) = 0;
        } else {
            at<u32>(slot, 0) = source[0];
            at<u32>(slot, 4) = source[1];
            at<u32>(slot, 8) = source[2];
            at<u32>(slot, 0xc) = source[3];
        }
    }
    return 1;
}

u32 __fastcall Table76e300_GetObjectDword(const u8 *object, void *, u32 id) {
    const u32 record = Table76e300_FindRecord(id);
    if (!record)
        return 0;
    return at<u32>(object, 0x24 + 12 * global<u32>(record + 8));
}

u8 __fastcall Table76e300_ObjectDwordSet(const u8 *object, void *, u32 id) {
    const u32 record = Table76e300_FindRecord(id);
    if (!record)
        return 0;
    return at<u32>(object, 0x28 + 12 * global<u32>(record + 8)) != 0;
}

void __fastcall CountedList_Clear(u8 *object) {
    u8 *sentinel = at<u8 *>(object, 4);
    u8 *node = at<u8 *>(sentinel, 0);
    while (node != sentinel) {
        u8 *erased = node;
        node = at<u8 *>(node, 0);
        at<u32>(at<u8 *>(erased, 4), 0) = at<u32>(erased, 0);
        at<u32>(at<u8 *>(erased, 0), 4) = at<u32>(erased, 4);
        CountedBuf_Reset(erased + 8, nullptr, 1);
        GameHeap_Free(erased);
        at<u32>(object, 8) -= 1;
    }
    GameHeap_Free(at<void *>(object, 4));
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
}

}  // extern "C"
