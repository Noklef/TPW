#include "streambuf.h"

#include <cstdint>
#include <initializer_list>

#include "gameheap.h"
#include "vcall.h"
#include "vwrap.h"

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

u32 *__fastcall Streambuf_Regrow(u8 *object, void *, u32 n) {
    s32 size = (s32)(n * 4);
    if (size < 0) size = 0;
    u8 *block = static_cast<u8 *>(GameHeap_Alloc((u32)size));
    u32 *out = reinterpret_cast<u32 *>(block + (n & 0xfffffffcu));
    u32 *to = out;
    const u32 *end = at<u32 *>(object, 0x20) + 1;
    for (const u32 *from = at<u32 *>(object, 0x10); from != end; ++from)
        *to++ = *from;
    GameHeap_Free(at<void *>(object, 0x24));
    at<u8 *>(object, 0x24) = block;
    at<u32>(object, 0x28) = n;
    return out;
}

u32 __fastcall Streambuf_Read(u8 *object, void *, u8 *destination, s32 count) {
    s32 total = 0;
    while (count > 0) {
        const u8 *p = *at<u8 **>(object, 0x1c);
        s32 room = 0;
        if (p && (room = *at<s32 *>(object, 0x2c)) > 0) {
            const s32 n = count < room ? count : room;
            for (s32 i = 0; i < n; ++i)
                destination[i] = p[i];
            total += n;
            count -= n;
            *at<s32 *>(object, 0x2c) -= n;
            destination += n;
            *at<u8 **>(object, 0x1c) += n;
        } else {
            const s32 c = vcall<s32>(object, 0x14);
            if (c == -1) break;
            *destination++ = (u8)c;
            ++total;
            --count;
        }
    }
    return (u32)total;
}

u32 __fastcall Streambuf_Write(u8 *object, void *, const u8 *source, s32 count) {
    s32 total = 0;
    while (count > 0) {
        u8 *p = *at<u8 **>(object, 0x20);
        s32 room = 0;
        if (p && (room = *at<s32 *>(object, 0x30)) > 0) {
            const s32 n = count < room ? count : room;
            for (s32 i = 0; i < n; ++i)
                p[i] = source[i];
            total += n;
            count -= n;
            *at<s32 *>(object, 0x30) -= n;
            source += n;
            *at<u8 **>(object, 0x20) += n;
        } else {
            if (vcall<s32>(object, 4, (u32)*source) == -1) break;
            ++source;
            ++total;
            --count;
        }
    }
    return (u32)total;
}

u8 __fastcall Eleven_Read(u8 *object, void *, u8 *stream) {
    for (u32 offset = 0; offset < 0x2c; offset += 4)
        Object_CallSlot8Product((u32)(uintptr_t)(object + offset), 4, 1, stream);
    return 1;
}

u32 __fastcall Wstreambuf_InitPointers(u8 *object) {
    static const u32 kSlots[6][2] = {{0xc, 0x4}, {0x10, 0x8}, {0x1c, 0x14}, {0x20, 0x18}, {0x2c, 0x24}, {0x30, 0x28}};
    for (const auto &slot : kSlots)
        at<u32>(object, slot[0]) = (u32)(uintptr_t)object + slot[1];
    for (u32 offset : {0x8u, 0x18u, 0x28u, 0x4u, 0x14u, 0x24u})
        at<u32>(object, offset) = 0;
    return (u32)(uintptr_t)object + 0x24;
}

u32 *__fastcall Wstreambuf_Setg(u8 *object, void *, u32 first, u32 next, u32 last) {
    *at<u32 *>(object, 0xc) = first;
    *at<u32 *>(object, 0x1c) = next;
    u32 *count = at<u32 *>(object, 0x2c);
    *count = (u32)((s32)(last - next) >> 1);
    return count;
}

u32 *__fastcall Wstreambuf_Setp(u8 *object, void *, u32 first, u32 last) {
    *at<u32 *>(object, 0x10) = first;
    *at<u32 *>(object, 0x20) = first;
    u32 *count = at<u32 *>(object, 0x30);
    *count = (u32)((s32)(last - first) >> 1);
    return count;
}
}  // extern "C"
