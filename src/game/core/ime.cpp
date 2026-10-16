#include "ime.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall Ime_Ctor(u8 *ime) {
    // The original's `push ecx` makes room for a local allocator it never sets, then copies its
    // byte (the pushed this's top byte) into each vector (sic).
    const u8 junk = (u8)((uintptr_t)ime >> 24);
    for (u32 v = 8; v < 0x48; v += 0x10) {
        at<u8>(ime, v) = junk;
        at<u32>(ime, v + 4) = 0;
        at<u32>(ime, v + 8) = 0;
        at<u32>(ime, v + 0xc) = 0;
    }
    at<u8>(ime, 0x48) = 0;
    at<u32>(ime, 0x4c) = 0;
    at<u32>(ime, 0x50) = 0;
    at<u32>(ime, 0x54) = 0xffffffff;
    at<u32>(ime, 0x5c) = 0;
    at<u32>(ime, 0) = 0x006fef04;
    return ime;
}

u32 *__fastcall ImeVector_Copy(void *, void *, const u32 *first, const u32 *last, u32 *dest) {
    for (; first != last; ++first, ++dest)
        if (dest) *dest = *first;
    return dest;
}

void __fastcall ImeVector_Fill(void *, void *, u32 *dest, u32 n, const u32 *value) {
    for (; n != 0; --n, ++dest)
        if (dest) *dest = *value;
}
}
