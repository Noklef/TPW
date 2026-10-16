#include "records16.h"

#include <cstdint>

#include "seqs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void Swap16(u8 *a, u8 *b) {
    for (u32 off = 0; off < 0x10; off += 4) {
        const u32 t = at<u32>(a, off);
        at<u32>(a, off) = at<u32>(b, off);
        at<u32>(b, off) = t;
    }
}

}  // namespace

extern "C" {

bool __cdecl Char_IsSpaceOrComma(const u16 *c) { return *c == 0x20 || *c == 0x2c; }

void __cdecl Records16_SwapMatchingBracket(u8 *first, u8 *last) {
    const u32 wanted = Char_BracketClass(reinterpret_cast<const u16 *>(first));
    s32 depth = 0;
    if (first >= last)
        return;
    for (u8 *p = first; p < last; p += 0x10) {
        const u32 cls = Char_BracketClass(reinterpret_cast<const u16 *>(p));
        if (cls == wanted)
            ++depth;
        else if (cls != 0)
            --depth;
        if (depth == 0) {
            Swap16(first, p);
            return;
        }
    }
}

u8 *__cdecl Records16_FindSpace(u8 *first, u8 *last) {
    for (u8 *p = first; p < last; p += 0x10) {
        if (Char_IsSpaceOrComma(reinterpret_cast<const u16 *>(p)))
            return p;
    }
    return last;
}

u8 *__cdecl Records16_FindNonSpace(u8 *first, u8 *last) {
    if ((uintptr_t)first > (uintptr_t)last - 0x20 || first >= last)
        return nullptr;
    for (u8 *p = first; p < last; p += 0x10) {
        if (!Char_IsSpaceOrComma(reinterpret_cast<const u16 *>(p)))
            return p;
    }
    return nullptr;
}

void __cdecl Records16_ReorderRtl(u8 *first, u8 *last) {
    if (!Records16_AnyInRange5b0(first, last) || last == first)
        return;
    while (at<u8>(last, (u32)-0xc) & 0x40) {
        last -= 0x10;
        if (last == first)
            break;
    }
    if (last == first)
        return;
    Records16_Reverse(first, last);
    u8 *run = Records16_FindNonSpace(first, last);
    while (run) {
        u8 *end = Records16_FindSpace(run, last);
        if (!Records16_AnyInRange5b0(run, end))
            Records16_Reverse(run, end);
        else if ((s32)Char_BracketClass(reinterpret_cast<const u16 *>(run)) > 0)
            Records16_SwapMatchingBracket(run, end);
        run = Records16_FindNonSpace(end, last);
    }
}

void __cdecl Records16_Reverse(u8 *first, u8 *last) {
    u8 *low = first;
    u8 *high = reinterpret_cast<u8 *>((uintptr_t)last - 0x10);
    if ((uintptr_t)high <= (uintptr_t)low)
        return;
    do {
        Swap16(low, high);
        low += 0x10;
        high -= 0x10;
    } while ((uintptr_t)high > (uintptr_t)low);
}

}  // extern "C"
