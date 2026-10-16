#include "methods.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall Obj_Get18Of14(const u8 *object) {
    const u8 *target = at<const u8 *>(object, 0x14);
    return target ? at<u32>(target, 0x18) : 0;
}

u32 *__fastcall Obj_MarkNodeIf14(u8 *object, void *, u32 *out, u8 **node, u32) {
    if (!at<u32>(object, 0x14)) {
        *out = 0xffffffff;
        return out;
    }
    at<u8>(*node, 0x1b) |= 0x40;
    at<u32>(*node, 4) = 1;
    *out = 0;
    return out;
}

u32 *__fastcall Obj_SetBit1At28(u8 *object, void *, u32 *out, u32 on) {
    const u32 flags = at<u32>(object, 0x28);
    if (!(flags & 1)) {
        *out = 0xffffffff;
        return out;
    }
    at<u32>(object, 0x28) = on ? flags | 2 : flags & ~2u;
    *out = 0;
    return out;
}

void __fastcall Obj_SetRateBits20(u8 *object, void *, u32 rate) {
    u8 bits = at<u8>(object, 0x20) & 0xe3;
    const u16 low = (u16)rate;
    bits |= low == 0x5622 ? 8 : low == 0xac44 ? 0x10 : 4;
    at<u8>(object, 0x20) = bits;
}

void __fastcall Obj_PutWord(u8 *object, void *, u32 word) {
    if (!at<u8>(object, 0x10) && at<u32>(object, 4) == at<u32>(object, 8))
        at<u8>(object, 0x10) = 1;
    if (!at<u8>(object, 0x10)) {
        *at<u16 *>(object, 4) = (u16)word;
        at<u32>(object, 4) += 2;
    }
}

u32 __fastcall Obj_ClearBit0If2c(u8 *object, void *, u32, u32 key, u32 flag) {
    if (key != at<u32>(object, 0x2c))
        return key;
    u32 flags = at<u32>(object, 0x30) & ~1u;
    at<u32>(object, 0x24) = 0;
    at<u32>(object, 0x30) = flags;
    if (!flag && !(flags & 0x40) && !(flags & 0x4800)) {
        flags |= 0x10;
        at<u32>(object, 0x30) = flags;
    }
    return flags;
}

u32 *__fastcall Obj_Pick40Or44(u8 *object, void *, u32 *out, const u8 *source) {
    at<u32>(object, 4) = (at<u32>(source, 0x4c) & 0x100) ? at<u32>(source, 0x44) : at<u32>(source, 0x40);
    *out = 0;
    return out;
}

u32 *__fastcall Obj_SetRateAtC(u8 *object, void *, u32 *out, u32 low) {
    const u32 rate = low ? 11025 : 22050;
    if (at<u32>(object, 0xc) == rate) {
        *out = 0xffffffff;
        return out;
    }
    at<u32>(object, 0xc) = rate;
    *out = 0;
    return out;
}

u32 __stdcall Obj_Get20Or2400(const u8 *object) {
    const u32 value = at<u32>(object, 0x20);
    return value ? value : 0x2400;
}

u8 *__cdecl Blocks16_Copy(const u8 *first, const u8 *last, u8 *dest) {
    for (const u8 *p = first; p != last; p += 0x10, dest += 0x10) {
        at<u32>(dest, 0) = at<u32>(p, 0);
        at<u32>(dest, 4) = at<u32>(p, 4);
        at<u32>(dest, 8) = at<u32>(p, 8);
        at<u32>(dest, 0xc) = at<u32>(p, 0xc);
    }
    return dest;
}

u8 *__fastcall Vector8_EraseTo(u8 *vector, void *, u8 *dest, const u8 *first) {
    const u8 *end = at<const u8 *>(vector, 8);
    u8 *to = dest;
    for (const u8 *p = first; p != end; p += 8, to += 8) {
        at<u32>(to, 0) = at<u32>(p, 0);
        at<u32>(to, 4) = at<u32>(p, 4);
    }
    at<u8 *>(vector, 8) = to;
    return dest;
}

u32 *__fastcall Obj_SetKindA0(u8 *object, void *, u32 *out, s32 kind) {
    if (kind == 2 || (kind > 3 && kind <= 5)) {
        at<s32>(object, 0xa0) = kind;
        *out = 0;
        return out;
    }
    *out = 0xffffffff;
    return out;
}

u8 *__fastcall Records_DataIfTerminated(const u8 *object, void *, u32 index) {
    if (index >= at<u32>(object, 8))
        return nullptr;
    const u8 *record = at<const u8 *const *>(object, 0xc)[index];
    const u32 head = at<u32>(record, 0);
    if ((u8)head != 1)
        return nullptr;
    const u32 words = ((head >> 8) + 3) >> 2;
    const u8 *data = record + 4;
    return at<u32>(data, 4 * words) == 0 ? const_cast<u8 *>(data) : nullptr;
}

}  // extern "C"
