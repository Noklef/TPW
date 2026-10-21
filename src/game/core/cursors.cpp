#include "cursors.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall Table_AddressPlus18(const u8 *object, void *, u32 index) {
    const u32 base = at<u32>(object, 8);
    if (!base || (u16)index > at<u16>(at<const u8 *>(object, 0xc), 6))
        return 0;
    return at<u32>(at<const u8 *>(object, 0x10), 4 * (index & 0xffff)) + base + 0x18;
}

u32 __fastcall Table_Address(const u8 *object, void *, u32 index) {
    const u32 base = at<u32>(object, 8);
    if (!base || (u16)index > at<u16>(at<const u8 *>(object, 0xc), 6))
        return 0;
    return at<u32>(at<const u8 *>(object, 0x10), 4 * (index & 0xffff)) + base;
}

u16 __fastcall Packed_Read(const u8 *object, void *, const u8 **cursor) {
    const u8 *p = *cursor;
    const u16 first = (u16)(p[0] + 0xffff);
    *cursor = p + 1;
    const u16 limit = at<u8>(at<const u8 *>(object, 0x14), 8);
    if (first < limit)
        return first;
    const u16 value = (u16)((u16)(first + 0x101) * 255u - limit * 0xfeu + p[1]);
    *cursor = p + 2;
    return value;
}

u32 __fastcall Nibble_Write(u8 *object, void *, u32 nibble) {
    if (!at<u8>(object, 5)) {
        if (at<u8>(object, 4)) {
            *at<u8 *>(object, 0) |= (u8)(nibble & 0xf);
            at<u8 *>(object, 0) += 1;
        } else {
            *at<u8 *>(object, 0) = (u8)((u8)nibble << 4);
        }
    }
    at<u8>(object, 4) = at<u8>(object, 4) == 0;
    return ++at<u32>(object, 8);
}

u8 *__fastcall Nibble_Next(u8 *object, void *, u8 *out, u32) {
    const u8 flag = at<u8>(object, 0);
    u8 *byte = at<u8 *>(object, 4);
    if (flag)
        at<u8 *>(object, 4) = byte + 1;
    at<u8>(object, 0) = flag == 0;
    at<u8 *>(out, 4) = byte;
    at<u8>(out, 0) = flag;
    return out;
}

u8 *__fastcall Bit_Next(u8 *object, void *, u8 *out, u32) {
    const u8 mask = at<u8>(object, 0);
    u8 *byte = at<u8 *>(object, 4);
    const u8 rotated = (u8)((mask >> 1) | (mask << 7));
    at<u8>(object, 0) = rotated;
    at<u8 *>(object, 4) = byte + (rotated == 0x80);
    at<u8 *>(out, 4) = byte;
    at<u8>(out, 0) = mask;
    return out;
}

u8 *__fastcall Cursor_CopyState(const u8 *object, void *, u8 *out) {
    const u32 byte = at<u32>(object, 4);
    at<u8>(out, 0) = at<u8>(object, 0);
    at<u32>(out, 4) = byte;
    return out;
}

}  // extern "C"
