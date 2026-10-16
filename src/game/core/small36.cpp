#include "small36.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The room left: the word behind the pointer, 31 bits sign extended.
s32 Room(const u32 *count) { return (s32)(*count << 1) >> 1; }

}  // namespace

extern "C" {

u32 __fastcall Wstreambuf_Read(u8 *object, void *, u16 *destination, s32 count) {
    s32 total = 0;
    while (count > 0) {
        const u16 *p = *at<u16 **>(object, 0x1c);
        s32 room = 0;
        if (p && (room = Room(at<u32 *>(object, 0x2c))) > 0) {
            const s32 n = count < room ? count : room;
            for (s32 i = 0; i < n; ++i)
                destination[i] = p[i];
            total += n;
            count -= n;
            *at<u32 *>(object, 0x2c) -= (u32)n;
            destination += n;
            *at<u8 **>(object, 0x1c) += 2 * n;
        } else {
            const u16 c = (u16)vcall<u32>(object, 0x14);
            if (c == 0xffff) break;
            *destination++ = c;
            ++total;
            --count;
        }
    }
    return (u32)total;
}

u32 __fastcall Wstreambuf_Write(u8 *object, void *, const u16 *source, s32 count) {
    s32 total = 0;
    while (count > 0) {
        u16 *p = *at<u16 **>(object, 0x20);
        s32 room = 0;
        if (p && (room = Room(at<u32 *>(object, 0x30))) > 0) {
            const s32 n = count < room ? count : room;
            for (s32 i = 0; i < n; ++i)
                p[i] = source[i];
            total += n;
            count -= n;
            *at<u32 *>(object, 0x30) -= (u32)n;
            source += n;
            *at<u8 **>(object, 0x20) += 2 * n;
        } else {
            const u32 argument = ((u32)(uintptr_t)p & 0xffff0000u) | *source;
            const u16 c = (u16)vcall<u32>(object, 4, argument);
            if (c == 0xffff) break;
            ++source;
            ++total;
            --count;
        }
    }
    return (u32)total;
}

}  // extern "C"
