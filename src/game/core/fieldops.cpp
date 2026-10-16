#include "fieldops.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall Obj_SetFirstFlag80(u8 *object, void *, u32 value) {
    at<u32>(object, 0x14) |= 0x80;
    at<u32>(object, 0) = value;
    return value;
}

u32 __fastcall Obj_SetSecondFlag8(u8 *object, void *, u32 value) {
    at<u32>(object, 0x14) |= 0x8;
    at<u32>(object, 4) = value;
    return value;
}

u32 __fastcall Obj_SetByte44Flag400(u8 *object, void *, u32 value) {
    at<u8>(object, 0x44) = (u8)value;
    return at<u32>(object, 0x30) |= 0x400;
}

u32 __fastcall Obj_Set242cTimes8(u8 *object, void *, u32 value) {
    value <<= 3;
    at<u32>(object, 0x242c) = value;
    at<u32>(object, 0x2430) = value;
    return value;
}

u32 __fastcall Obj_Set54And58Flag8(u8 *object, void *, u32 a, u32 b) {
    at<u32>(object, 0x54) = a;
    at<u32>(object, 0x58) = b;
    return at<u32>(object, 0x4c) |= 0x8;
}

u32 __fastcall Obj_SampleRate20(const u8 *object) {
    const u8 flags = at<u8>(object, 0x20);
    if (flags & 0x8)
        return 22050;
    return (flags & 0x10) ? 44100 : 11025;
}

u32 __fastcall Obj_WordFrom6(const u8 *object) {
    if (!at<u32>(object, 8))
        return 0;
    const u32 target = at<u32>(object, 0xc);
    return (target & 0xffff0000) | at<u16>(reinterpret_cast<const u8 *>(target), 6);
}

u32 __fastcall Obj_ByteFrom5(const u8 *object) {
    if (!at<u32>(object, 8))
        return 0;
    const u32 target = at<u32>(object, 0xc);
    return (target & 0xffff0000) | at<u8>(reinterpret_cast<const u8 *>(target), 5);
}

void __fastcall Chain24_Promote400To100(const u8 *object) {
    for (u8 *node = at<u8 *>(object, 0x24); node; node = at<u8 *>(node, 0x34)) {
        u32 flags = at<u32>(node, 0x30);
        if (flags & 0x400) {
            flags = (flags & ~0x400u) | 0x100;
            at<u32>(node, 0x30) = flags;
        }
    }
}

u8 *__fastcall Chain20_Find2c(const u8 *object, void *, u32 value) {
    for (u8 *node = at<u8 *>(object, 0x20); node; node = at<u8 *>(node, 0x34)) {
        if (at<u32>(node, 0x2c) == value)
            return node;
    }
    return nullptr;
}

u32 __fastcall Obj_Set4cMin1e(u8 *object, void *, u32 value) {
    if (value >= 0x1e)
        value = 0x1e;
    at<u32>(object, 0x4c) = value;
    return value;
}

u8 *__fastcall Chain20_PushFront(u8 *object, void *, u8 *node) {
    at<u8 *>(node, 0x34) = at<u8 *>(object, 0x20);
    at<u32>(node, 0x38) = 0;
    if (u8 *head = at<u8 *>(object, 0x20))
        at<u8 *>(head, 0x38) = node;
    at<u8 *>(object, 0x20) = node;
    return node;
}

u8 *__fastcall Chain24_PushFront(u8 *object, void *, u8 *node) {
    at<u8 *>(node, 0x34) = at<u8 *>(object, 0x24);
    at<u32>(node, 0x38) = 0;
    if (u8 *head = at<u8 *>(object, 0x24))
        at<u8 *>(head, 0x38) = node;
    at<u8 *>(object, 0x24) = node;
    return node;
}

u32 *__fastcall Bytes44_Set(u8 *object, void *, u32 *status, u32 index, u32 value) {
    if ((u8)index >= 4) {
        *status = 0xffffffff;
        return status;
    }
    at<u8>(object, 0x44 + (index & 0xff)) = (u8)value;
    *status = 0;
    return status;
}

bool __fastcall Obj_PastEnd(const u8 *object, void *, u32 value) {
    return !(at<u8>(object, 0x30) & 2) && value > at<u32>(object, 0x1c) + at<u32>(object, 0x3c);
}

u32 __fastcall Array_AtOrZero(const u8 *object, void *, u32 index) {
    const u8 *array = at<const u8 *>(object, 8);
    return array && index < at<u32>(object, 4) ? at<u32>(array, 4 * index) : 0;
}

u32 __fastcall Obj_Set94_98_Flag40000(u8 *object, void *, u32 a, u32 b) {
    at<u32>(object, 0x14) |= 0x40000;
    at<u32>(object, 0x98) = b;
    at<u32>(object, 0x94) = a;
    return a;
}

u32 __fastcall Nibble_Select(const u8 *object) {
    const u8 value = *at<const u8 *>(object, 4);
    return at<u8>(object, 0) ? value & 0xf : value >> 4;
}

u8 *__fastcall Entries8_Pop(u8 *object) {
    s32 count = at<s32>(object, 0x200);
    if (count <= 0)
        return nullptr;
    at<s32>(object, 0x200) = --count;
    return object + 8 * (u32)count;
}

u32 __fastcall Offsets_Resolve(const u8 *object, void *, u32 index) {
    if (index > at<u32>(object, 0xc))
        return 0;
    const u32 base = at<u32>(object, 8);
    return at<u32>(reinterpret_cast<const u8 *>(base), 4 * index) + base + 8;
}

u32 *__fastcall Targets3_Set(u8 *object, void *, u32 a, u32 b, u32 c) {
    *at<u32 *>(object, 0xc) = a;
    *at<u32 *>(object, 0x1c) = b;
    u32 *last = at<u32 *>(object, 0x2c);
    *last = c - b;
    return last;
}

u32 *__fastcall Targets2_Set(u8 *object, void *, u32 a, u32 b) {
    *at<u32 *>(object, 0x10) = a;
    *at<u32 *>(object, 0x20) = a;
    u32 *last = at<u32 *>(object, 0x30);
    *last = b - a;
    return last;
}

}  // extern "C"
