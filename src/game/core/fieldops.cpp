#include "fieldops.h"

#include <cstdint>
#include <initializer_list>

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

u8 *__fastcall Chain20_MoveTo24(u8 *object, void *, u8 *node) {
    if (u8 *prev = at<u8 *>(node, 0x38))
        at<u8 *>(prev, 0x34) = at<u8 *>(node, 0x34);
    else
        at<u8 *>(object, 0x20) = at<u8 *>(node, 0x34);
    if (u8 *next = at<u8 *>(node, 0x34))
        at<u8 *>(next, 0x38) = at<u8 *>(node, 0x38);
    return Chain24_PushFront(object, nullptr, node);
}

u8 *__fastcall Chain24_MoveTo20(u8 *object, void *, u8 *node) {
    if (u8 *prev = at<u8 *>(node, 0x38))
        at<u8 *>(prev, 0x34) = at<u8 *>(node, 0x34);
    else
        at<u8 *>(object, 0x24) = at<u8 *>(node, 0x34);
    if (u8 *next = at<u8 *>(node, 0x34))
        at<u8 *>(next, 0x38) = at<u8 *>(node, 0x38);
    return Chain20_PushFront(object, nullptr, node);
}

u8 *__fastcall Pages100_EntryAt(const u8 *object, void *, u32 index) {
    index &= 0xffff;
    u32 hops = index / 100;
    const u32 rest = index % 100;
    u8 *page = at<u8 *>(object, 0x1c);
    if (!page)
        return nullptr;
    for (;;) {
        if (hops-- == 0)
            break;
        page = at<u8 *>(page, 8);
        if (!page)
            break;
    }
    return page ? page + rest * 0x50 + 0xc : nullptr;
}

u32 __fastcall Top_Insert(u8 *object, void *, u8 *node) {
    if (at<u32>(object, 0x48) >= at<u32>(node, 0x20))
        return at<u32>(object, 0x48);
    u32 i = 0;
    if (at<u32>(object, 0x4c) != 0 && node) {
        do {
            u8 **slot = &at<u8 *>(at<u8 *>(object, 0x44), 4 * i);
            u8 *entry = *slot;
            if (!entry || at<u32>(entry, 0x20) < at<u32>(node, 0x20)) {
                *slot = node;
                node = entry;
            }
            ++i;
        } while (i < at<u32>(object, 0x4c) && node);
    }
    const u8 *last = at<u8 *const *>(object, 0x44)[at<u32>(object, 0x4c) - 1];
    const u32 key = last ? at<u32>(last, 0x20) : 0;
    at<u32>(object, 0x48) = key;
    return key;
}

u8 *__fastcall Class70a7e0_Ctor(u8 *object) {
    for (u32 off : {4u, 8u, 0xcu, 0x10u, 0x14u, 0x18u, 0x1cu, 0x20u, 0x24u, 0x28u, 0x30u, 0x3cu, 0x40u})
        at<u32>(object, off) = 0;
    at<u8>(object, 0x45) = 0xff;
    at<u8>(object, 0x46) = 0xff;
    at<u32>(object, 0x4c) = 0;
    at<u32>(object, 0) = 0x0070a7e0;
    return object;
}

u32 *__fastcall Target24_SetBit400(const u8 *object, void *, u32 *status, u32 flag) {
    u8 *target = at<u8 *>(object, 0x24);
    if (!target) {
        *status = 0xffffffff;
        return status;
    }
    u32 bits = at<u32>(target, 0x28);
    bits = flag ? bits | 0x400 : bits & 0xfffffbff;
    at<u32>(target, 0x28) = bits;
    if (!flag) {
        if (const u8 *owner = at<const u8 *>(object, 0x20))
            Chain24_Promote400To100(owner);
    }
    *status = 0;
    return status;
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

u16 __stdcall WordConstffff_4f06a0(u32 v0) {
    return 0xffff;
}

u16 __cdecl WordConstffff_4f06b0() {
    return 0xffff;
}

u32 __fastcall CopyTo_20_6a8cd0(u8 *o, void *, u32 v0) {
    at<u32>((void *)(uintptr_t)v0, 0) = at<u32>(o, 0x20);
    return v0;
}

u32 __fastcall CopyTo_24_6a8ce0(u8 *o, void *, u32 v0) {
    at<u32>((void *)(uintptr_t)v0, 0) = at<u32>(o, 0x24);
    return v0;
}

u32 __stdcall StoreThrough_ffffffff_6b4a00(u32 v0, u32 v1, u32 v2) {
    at<u32>((void *)(uintptr_t)v0, 0) = 0xffffffff;
    return v0;
}

u32 __fastcall SetThenFlag50_4c_6b9c70(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x50) = v0;
    at<u32>(o, 0x4c) |= 0x4;
    return at<u32>(o, 0x4c);
}

u32 __fastcall SetThenFlag60_4c_6b9cb0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x60) = v0;
    at<u32>(o, 0x4c) |= 0x20;
    return at<u32>(o, 0x4c);
}

u32 __fastcall SetThenFlag40_4c_6b9cf0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x40) = v0;
    at<u32>(o, 0x4c) |= 0x80;
    return at<u32>(o, 0x4c);
}

u32 __fastcall SetThenFlag44_4c_6b9d10(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x44) = v0;
    at<u32>(o, 0x4c) |= 0x100;
    return at<u32>(o, 0x4c);
}

u32 __fastcall SetThenFlag68_4c_6b9d30(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x68) = v0;
    at<u32>(o, 0x4c) |= 0x200;
    return at<u32>(o, 0x4c);
}

u32 __fastcall SetThenFlag48_14_6c5090(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x48) = v0;
    at<u32>(o, 0x14) |= 0x1000;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag8_14_6c50b0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x8) = v0;
    at<u32>(o, 0x14) |= 0x4;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag9c_14_6c50d0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x9c) = v0;
    at<u32>(o, 0x14) |= 0x80000;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag4c_14_6c50f0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x4c) = v0;
    at<u32>(o, 0x14) |= 0x800;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag44_14_6c5110(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x44) = v0;
    at<u32>(o, 0x14) |= 0x40;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag28_14_6c5130(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x28) = v0;
    at<u32>(o, 0x14) |= 0x2;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag2c_14_6c5150(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x2c) = v0;
    at<u32>(o, 0x14) |= 0x400;
    return at<u32>(o, 0x14);
}

u32 __fastcall SetThenFlag24_14_6c5170(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x24) = v0;
    at<u32>(o, 0x14) |= 0x20;
    return at<u32>(o, 0x14);
}

u32 __fastcall FlagThenSet14_5c_6c51d0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x14) |= 0x10000;
    at<u32>(o, 0x5c) = v0;
    return v0;
}

u32 __fastcall FlagThenSet14_58_6c5220(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x14) |= 0x200000;
    at<u32>(o, 0x58) = v0;
    return v0;
}

u32 __stdcall StoreThrough_ffffffff_6d2790(u32 v0, u32 v1, u32 v2, u32 v3, u32 v4) {
    at<u32>((void *)(uintptr_t)v0, 0) = 0xffffffff;
    return v0;
}

u32 __stdcall StoreThrough_ffffffff_6d27a0(u32 v0) {
    at<u32>((void *)(uintptr_t)v0, 0) = 0xffffffff;
    return v0;
}

u32 __stdcall StoreThrough_ffffffff_6d29e0(u32 v0, u32 v1, u32 v2, u32 v3) {
    at<u32>((void *)(uintptr_t)v0, 0) = 0xffffffff;
    return v0;
}
}  // extern "C"
