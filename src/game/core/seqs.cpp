#include "seqs.h"

#include <cstdint>

#include "../files/file.h"
#include "gameheap.h"
#include "movs.h"

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

u32 __stdcall Path_AfterLastBackslash(const char **text, u32 length) {
    u32 i = length - 1;
    if (i == 0)
        return 0;
    const char *s = *text;
    do {
        if (s[i] == '\\') {
            *text = s + i + 1;
            return i;
        }
    } while (--i != 0);
    return 0;
}

s32 __stdcall SignedBytes_Compare(const s8 *first1, const s8 *last1, const s8 *first2, const s8 *last2) {
    for (; first1 != last1; ++first1, ++first2) {
        if (first2 == last2)
            return 1;
        if (*first2 > *first1)
            return -1;
        if (*first2 < *first1)
            return 1;
    }
    return first2 == last2 ? 0 : -1;
}

namespace {

// The step both loops of ByteBuf_Write share: sets the full flag when the cursor has reached the end; true when it is set.
bool ByteBuf_Full(u8 *buffer) {
    if (!at<u8>(buffer, 0x10) && at<u32>(buffer, 4) == at<u32>(buffer, 8))
        at<u8>(buffer, 0x10) = 1;
    return at<u8>(buffer, 0x10) != 0;
}

}  // namespace

void __fastcall ByteBuf_Write(u8 *buffer, void *, const u8 *text, s32 count) {
    if (at<u32>(buffer, 4) >= at<u32>(buffer, 8))
        return;
    if (count == -1) {
        if (!*text)
            return;
        do {
            if (ByteBuf_Full(buffer))
                return;
            *at<u8 *>(buffer, 4) = *text++;
            at<u8 *>(buffer, 4) += 1;
        } while (*text);
        return;
    }
    u32 n = (u32)count;
    if (!n)
        return;
    do {
        if (ByteBuf_Full(buffer))
            return;
        *at<u8 *>(buffer, 4) = *text++;
        at<u8 *>(buffer, 4) += 1;
    } while (--n);
}

u8 *__fastcall Table12_Find(const u8 *table, void *, u32 key) {
    const u8 *low = at<const u8 *>(table, 8) - 12;
    const u8 *high = at<const u8 *>(table, 0xc) + 12;
    for (s32 count = (s32)(high - low) / 12; count > 1; count = (s32)(high - low) / 12) {
        const u8 *middle = low + 12 * (u32)(count / 2);
        const u32 found = at<u32>(middle, 0);
        if (key == found)
            return const_cast<u8 *>(middle);
        if (key < found)
            high = middle;
        else
            low = middle;
    }
    return nullptr;
}

u32 __stdcall Mem_CopyForward(void *dest, const void *source, u32 count) {
    Rep_Movs(dest, source, count);
    return count;
}

u8 *__cdecl Blocks8_Copy(const u8 *first, const u8 *last, u8 *dest) {
    for (const u8 *p = first; p != last; p += 8, dest += 8) {
        at<u32>(dest, 0) = at<u32>(p, 0);
        at<u32>(dest, 4) = at<u32>(p, 4);
    }
    return dest;
}

void __cdecl Word_CopyTo(u16 *dest, const u16 *src) {
    if (dest)
        *dest = *src;
}

u32 *__cdecl Dwords_Copy(const u32 *first, const u32 *last, u32 *dest) {
    for (; first != last; first++, dest++)
        *dest = *first;
    return dest;
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

u32 __stdcall Quad_Push(const u32 *source, u32, u32, u8 *object) {
    const s32 count = at<s32>(object, 0x788);
    if (count < 4) {
        u8 *slot = object + 0x448 + 16 * (u32)count;
        at<s32>(object, 0x788) = count + 1;
        if (!source) {
            at<u32>(slot, 0) = 0;
            at<u32>(slot, 4) = 0;
            at<u32>(slot, 8) = 0;
            at<u32>(slot, 0xc) = 0;
        } else {
            at<u32>(slot, 0) = source[0];
            at<u32>(slot, 4) = source[1];
            at<u32>(slot, 8) = source[2];
            at<u32>(slot, 0xc) = source[3];
        }
    }
    return 1;
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

u32 __cdecl Dwords_Zero(u32 *to, u32 count) {
    for (u32 i = 0; i < count; ++i)
        to[i] = 0;
    return 0;
}

void __fastcall Bands_SortValues(const u8 *object, void *, u32 *values, s32 lo, s32 hi) {
    for (;;) {
        const u32 pivot = values[(s32)((u32)lo + (u32)hi) >> 1];
        s32 i = lo, j = hi;
        do {
            while (i <= hi && values[i] < pivot)
                ++i;
            if (i > lo) {  // (the original tests i, not j, against lo, sic)
                while (values[j] > pivot)
                    --j;
            }
            if (i > j)
                break;
            const u32 swap = values[i];
            values[i] = values[j];
            values[j] = swap;
            ++i;
            --j;
        } while (i <= j);
        if (lo < j)
            Bands_SortValues(object, nullptr, values, lo, j);
        if (i >= hi)
            return;
        lo = i;
    }
}
}  // extern "C"
