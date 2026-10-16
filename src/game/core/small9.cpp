#include "small9.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void ReleaseCounted(u8 *buffer) {
    const u8 uses = buffer[-1];
    if (uses == 0 || uses == 0xff)
        GameHeap_Free(buffer - 2);
    else
        buffer[-1] = (u8)(uses - 1);
}

void ClearCountedVector(u8 *object, u32 begin, u32 end, u32 capacity) {
    u8 *entry = at<u8 *>(object, begin);
    u8 *last = at<u8 *>(object, end);
    while (entry != last) {
        if (at<u8 *>(entry, 4))
            ReleaseCounted(at<u8 *>(entry, 4));
        at<u32>(entry, 4) = 0;
        at<u32>(entry, 8) = 0;
        at<u32>(entry, 0xc) = 0;
        entry += 0x10;
    }
    GameHeap_Free(at<void *>(object, begin));
    at<u32>(object, begin) = 0;
    at<u32>(object, end) = 0;
    at<u32>(object, capacity) = 0;
}

u8 *Class7028_Ctor(u8 *object, u32 vtable, u32 memberVtable) {
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
        return object;
    }
    at<u32>(object, 0x2c) = 0;
    return object;
}

}  // namespace

extern "C" {

u32 __fastcall Person_HasWord(const u8 *object, void *, u32 word) {
    const u8 kind = at<u8>(object, 2);
    if (kind == 4)
        return at<u16>(object, 0x218) == (u16)word;
    if (kind == 5)
        return at<u16>(object, 0x21a) == (u16)word;
    return 0;
}

void __stdcall TextExtent_Zero(u32 *out, const u8 *) {
    global<u32>(0x007cf100) = 0;
    global<u32>(0x007cf104) = 0;
    out[0] = global<u32>(0x007cf100);
    out[1] = global<u32>(0x007cf104);
}

u8 *__fastcall Block50_Ctor(u8 *object) {
    u8 *block = static_cast<u8 *>(GameHeap_Alloc(0x50));
    if (block) {
        for (u32 i = 0; i < 10; ++i) {
            at<u32>(block, 8 * i) = 0;
            at<u32>(block, 8 * i + 4) = 0;
        }
        at<u8 *>(object, 0) = block;
        at<u8 *>(object, 4) = block;
        at<u8 *>(object, 8) = block + 0x280;
        return object;
    }
    at<u32>(object, 0) = 0;
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0x280;
    return object;
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

void __fastcall Lnd_SetTileSize(u8 *object, void *, u32 size) {
    at<u8>(object, 0) = (u8)size;
    if (size == 0x40) {
        at<u8>(object, 1) = 0;
    } else if (size == 0x20) {
        at<u8>(object, 1) = 1;
    } else if (size == 0x10) {
        at<u8>(object, 1) = 2;
    } else {
        at<u8>(object, 0) = 0x20;
        at<u8>(object, 1) = 1;
    }
    if (global<u32>(0x008798c8))
        GameHeap_Free(reinterpret_cast<void *>(global<u32>(0x008798c8)));
    const u32 side = at<u8>(object, 0);
    global<u32>(0x008798c8) = (u32)(uintptr_t)GameHeap_Alloc(side * side * 4);
}

u8 *__fastcall Class702880_Ctor(u8 *object) { return Class7028_Ctor(object, 0x00702880, 0x007028a8); }

u8 *__fastcall Class702890_Ctor(u8 *object) { return Class7028_Ctor(object, 0x00702890, 0x007028ac); }

s32 __fastcall ByteString_Compare(const u8 *object, void *, const u8 *other) {
    const u32 otherLength = at<u32>(other, 8);
    const u8 *otherText = at<const u8 *>(other, 4);
    if (!otherText)
        otherText = reinterpret_cast<const u8 *>(0x006fe5b0);
    const u32 length = at<u32>(object, 8);
    const u8 *text = at<const u8 *>(object, 4);
    const u32 common = length < otherLength ? length : otherLength;
    for (u32 i = 0; i < common; ++i) {
        if (text[i] != otherText[i])
            return text[i] < otherText[i] ? -1 : 1;
    }
    if (length >= otherLength)
        return length != otherLength ? 1 : 0;
    return -1;
}

void __fastcall CountedVector14_Clear(u8 *object) { ClearCountedVector(object, 0x14, 0x18, 0x1c); }

void __fastcall CountedVector4_Clear(u8 *object) { ClearCountedVector(object, 4, 8, 0xc); }

}  // extern "C"
