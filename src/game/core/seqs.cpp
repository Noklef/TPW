#include "seqs.h"

#include <cstdint>

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

}  // extern "C"
