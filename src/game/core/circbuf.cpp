#include "circbuf.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall CircBufU16_3_Ctor(u8 *buf) {
    at<u16>(buf, 0) = 0;
    at<u32>(buf, 8) = 0xffffffff;
    at<u32>(buf, 0xc) = 3;
    at<u8>(buf, 0x10) = 0;
    return buf;
}

u8 *__fastcall CircBufU8_50_Ctor(u8 *buf) {
    at<u8>(buf, 0) = 0;
    at<u32>(buf, 0x34) = 0xffffffff;
    at<u32>(buf, 0x38) = 50;
    at<u8>(buf, 0x3c) = 0;
    return buf;
}

namespace {

u8 *u32_ctor(u8 *buf, u32 size) {
    at<u32>(buf, 0) = 0;
    at<u32>(buf, 4 + size * 4) = 0xffffffff;
    at<u32>(buf, 8 + size * 4) = size;
    at<u8>(buf, 0xc + size * 4) = 0;
    return buf;
}

}  // namespace

u8 *__fastcall CircBufU32_30_Ctor(u8 *buf) { return u32_ctor(buf, 30); }
u8 *__fastcall CircBufU32_144_Ctor(u8 *buf) { return u32_ctor(buf, 144); }
u8 *__fastcall CircBufU32_50_Ctor(u8 *buf) { return u32_ctor(buf, 50); }
u8 *__fastcall CircBufU32_5_Ctor(u8 *buf) { return u32_ctor(buf, 5); }

void __fastcall CircBufU32_144_Push(u8 *buf, void *, u32 value) {
    const s32 size = at<s32>(buf, 0x248);
    const s32 cursor = at<s32>(buf, 0x244) + 1;
    at<s32>(buf, 0x244) = cursor;
    if (cursor >= size) {
        at<s32>(buf, 0x244) = 0;
        at<u8>(buf, 0x24c) = 1;
    }
    at<u32>(buf, 4 + (u32)at<s32>(buf, 0x244) * 4) = value;
}

u8 __fastcall CircBufU32_30_Get(const u8 *buf, void *, u32 *out, s32 back) {
    const s32 size = at<s32>(buf, 0x80);
    if (back >= size) return 0;
    const s32 cursor = at<s32>(buf, 0x7c);
    if (back <= cursor) {
        *out = at<u32>(buf, 4 + ((u32)cursor - (u32)back) * 4);
        return 1;
    }
    if (!at<u8>(buf, 0x84)) return 0;
    *out = at<u32>(buf, 4 + ((u32)cursor - (u32)back + (u32)size) * 4);
    return 1;
}
}
