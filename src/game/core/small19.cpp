#include "small19.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall Record_CopyWithText(u8 *dest, void *, const u8 *source) {
    for (u32 offset = 0; offset < 0x20; offset += 4)
        at<u32>(dest, offset) = at<u32>(source, offset);
    const u8 *from = source + 0x24;
    u8 *to = dest + 0x24;
    u32 length = 0;
    while (from[length])
        ++length;
    for (u32 i = 0; i <= length; ++i)
        to[i] = from[i];
    at<u32>(dest, 0x20) = at<u32>(source, 0x20) - (u32)(uintptr_t)source + (u32)(uintptr_t)dest;
    return dest;
}

u32 __fastcall Bands_AnyMatch(const u8 *object, void *, u32 packed) {
    const s32 b0 = packed & 0xff, b1 = (packed >> 8) & 0xff, b2 = (packed >> 16) & 0xff, b3 = (packed >> 24) & 0xff;
    if (at<s32>(object, 0x128) > b0)
        return 0;
    if (at<s32>(object, 0x12c) < b1)
        return 0;
    if (b1 > b0)
        return 0;
    for (s32 i = b1; i <= b0; ++i) {
        if (at<s8>(object, 0x130 + i) <= b2 && at<s8>(object, 0x1b0 + i) >= b3)
            return 1;
    }
    return 0;
}

void __stdcall Bytes4_Permute(u8 *p, const u16 *flags) {
    if (!(*flags & 0x78))
        return;
    if (*flags & 0x40) {
        const u8 a = p[1], b = p[3];
        p[1] = b;
        p[3] = a;
    }
    const u16 f = *flags;
    if (f & 8) {
        const u8 p0 = p[0], p1 = p[1], p2 = p[2], p3 = p[3];
        p[3] = p2;
        p[2] = p1;
        p[1] = p0;
        p[0] = p3;
    } else if (f & 0x10) {
        const u8 p0 = p[0], p1 = p[1], p2 = p[2], p3 = p[3];
        p[0] = p2;
        p[2] = p0;
        p[1] = p3;
        p[3] = p1;
    } else if (f & 0x20) {
        const u8 p0 = p[0], p1 = p[1], p2 = p[2], p3 = p[3];
        p[0] = p1;
        p[1] = p2;
        p[2] = p3;
        p[3] = p0;
    }
}

}  // extern "C"
