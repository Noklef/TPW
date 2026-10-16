#include "small.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __cdecl Word_CopyTo(u16 *dest, const u16 *src) {
    if (dest)
        *dest = *src;
}

void __fastcall Vtbl703748_Set(u8 *object) { at<u32>(object, 0) = 0x00703748; }

void __fastcall Vtbl703770_Set(u8 *object) { at<u32>(object, 0) = 0x00703770; }

u8 *__fastcall Class703770_Ctor(u8 *object) {
    at<u32>(object, 0) = 0x00703770;
    at<u32>(object, 4) = 1;
    return object;
}

u32 __fastcall Obj_Get64Plus10(const u8 *object) { return at<u32>(object, 0x64) + 0x10; }

void __cdecl Stub_Ret2() {}

u32 __fastcall Obj_Get3c(const u8 *object) { return at<u32>(object, 0x3c); }

u32 *__cdecl Dwords_Copy(const u32 *first, const u32 *last, u32 *dest) {
    for (; first != last; first++, dest++)
        *dest = *first;
    return dest;
}

u8 *__fastcall Class703c80_Ctor(u8 *object) {
    at<u32>(object, 4) = 5;
    at<u32>(object, 8) = 5;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x10) = 1;
    at<u32>(object, 0x14) = 0xffffffff;
    at<u32>(object, 0x18) = 0xffffffff;
    at<u32>(object, 0x1c) = 0xffffffff;
    at<u32>(object, 0) = 0x00703c80;
    return object;
}

u32 __fastcall Obj_Take(u8 *object, void *, const u8 *source) {
    at<u32>(object, 0x14) = 0;
    at<u16>(object, 0xe) = at<u16>(source, 0xc);
    const u32 value = at<u32>(source, 0x10);
    at<u32>(object, 0x10) = value;
    at<u8>(object, 0xc) = 3;
    return value;
}

const u8 *__fastcall Obj_TakeWords(u8 *object, void *, const u8 *source) {
    at<u32>(object, 0x14) = at<u32>(source, 0x20);
    at<u8>(object, 0xd) = at<u8>(source, 0x10);
    at<u16>(object, 0xe) = (u16)(at<u8>(object, 0x14) & 8);
    at<u8>(object, 0xc) = 1;
    at<u32>(object, 0x10) = ((u32)at<u16>(source, 0x18) << 16) | at<u16>(source, 0x14);
    return source;
}

u32 __fastcall Obj_Copy4At34(u8 *object, void *, const u32 *source) {
    if (!source)
        return 0;
    for (u32 i = 0; i < 4; i++)
        at<u32>(object, 0x34 + 4 * i) = source[i];
    return source[3];
}

u32 __cdecl Bits_Reverse(u32 code, s32 length) {
    u32 result = 0;
    do {
        result |= code & 1;
        code >>= 1;
        result <<= 1;
    } while (--length > 0);
    return result >> 1;
}

u8 *__cdecl Codec631280_Init(u8 *object) {
    at<u32>(object, 0) = 0x6312d0;
    at<u32>(object, 4) = 0x631340;
    at<u32>(object, 8) = 0x631300;
    at<u32>(object, 0xc) = 0x631380;
    at<u32>(object, 0x10) = 0x631440;
    at<u32>(object, 0x68) = 0;
    at<u32>(object, 0x6c) = 0;
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 0x70) = 0x7039d0;
    at<u32>(object, 0x74) = 0x7b;
    at<u32>(object, 0x78) = 0;
    at<u32>(object, 0x7c) = 0;
    at<u32>(object, 0x80) = 0;
    return object;
}

s32 __fastcall WString_Compare(const u8 *self, void *, const u8 *other) {
    const u32 otherLength = at<u32>(other, 8), selfLength = at<u32>(self, 8);
    const u16 *b = at<const u16 *>(other, 4);
    if (!b)
        b = reinterpret_cast<const u16 *>(0x006fd6c4);
    const u16 *a = at<const u16 *>(self, 4);
    for (u32 n = selfLength < otherLength ? selfLength : otherLength; n; n--, a++, b++) {
        if (*a != *b)
            return *a < *b ? -1 : 1;
    }
    if (selfLength < otherLength)
        return -1;
    return selfLength != otherLength ? 1 : 0;
}

u32 __fastcall Obj_GetWordPair(const u8 *object, void *, u32 *a, u32 *b) {
    if (at<u16>(object, 0x44) == 0 || !a || !b)
        return 0;
    *a = at<u16>(object, 0xa8);
    *b = at<u16>(object, 0xaa);
    return at<u32>(object, 0xa0);
}

void __cdecl EntryStack_Clear() {
    u32 &top = *reinterpret_cast<u32 *>(0x007cf07c);
    const u32 &base = *reinterpret_cast<u32 *>(0x007cf078);
    if (top <= base)
        return;
    do {
        at<u32>(reinterpret_cast<void *>(top), 0) = 0;
        at<u32>(reinterpret_cast<void *>(top), 4) = 0;
        top -= 8;
    } while (top > base);
}

u32 __fastcall Obj_Set10(u8 *object, void *, u32 value) {
    at<u32>(object, 0x10) = value;
    return value;
}

s32 __cdecl DivCeil(s32 a, s32 b) { return (s32)((u32)a + (u32)b - 1) / b; }

u32 __cdecl Const_C0000() { return 0xc0000; }

u32 __cdecl Const_1000000() { return 1000000; }
}
