#include "seqs.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 Rotl8(u32 v) { return (v << 8) | (v >> 24); }

}  // namespace

extern "C" {

u32 __stdcall Checksum_Words(const u16 *first, const u16 *last) {
    u32 sum = 0;
    for (const u16 *p = first; p != last; ++p)
        sum = *p + Rotl8(sum);
    return sum;
}

u32 __stdcall Checksum_SignedBytes(const s8 *first, const s8 *last) {
    u32 sum = 0;
    for (const s8 *p = first; p != last; ++p)
        sum = (u32)(s32)*p + Rotl8(sum);
    return sum;
}

u32 __cdecl Char_BracketClass(const u16 *c) { return *c == 0x28 ? 1 : *c == 0x29 ? 2 : 0; }

bool __cdecl Records16_AnyInRange5b0(const u8 *first, const u8 *last) {
    if (first >= last)
        return false;
    const u8 *p = first;
    do {
        const u16 c = at<u16>(p, 0);
        if (c > 0x5b0 && c < 0x5eb)
            return true;
        p += 0x10;
    } while (p < last);
    return false;
}

s32 __fastcall Vector8_Count4(const u8 *object) {
    const u8 *begin = at<const u8 *>(object, 4);
    return begin ? (s32)(at<const u8 *>(object, 8) - begin) >> 3 : 0;
}

s32 __fastcall Vector8_Count88(const u8 *object) {
    const u8 *begin = at<const u8 *>(object, 0x88);
    return begin ? (s32)(at<const u8 *>(object, 0x8c) - begin) >> 3 : 0;
}

u8 *__cdecl Blocks8_Copy(const u8 *first, const u8 *last, u8 *dest) {
    for (const u8 *p = first; p != last; p += 8, dest += 8) {
        at<u32>(dest, 0) = at<u32>(p, 0);
        at<u32>(dest, 4) = at<u32>(p, 4);
    }
    return dest;
}

}  // extern "C"
