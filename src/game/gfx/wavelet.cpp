#include "wavelet.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __stdcall WaveletCodec_PutCoefficient(u32 value, u8 **cursor) {
    s16 v = (s16)value;
    if (v >= -0x7f && v <= 0x7f) {
        **cursor = (u8)v;
        ++*cursor;
        return;
    }
    **cursor = 0x80;
    u8 *word = *cursor + 1;
    *cursor = word;
    *reinterpret_cast<u16 *>(word) = (u16)v;
    *cursor += 2;
}

u32 __cdecl Wavelet_ReadBits(u32 count) {
    u32 held = global<u32>(0x00aa67f8);
    if (held < count) {
        const u32 position = global<u32>(0x00be6800);
        const u32 word = at<u32>(reinterpret_cast<const void *>(position), global<u32>(0x00e74a40)) & 0xffffu;
        global<u32>(0x00be6800) = position + 2;
        global<u32>(0x00f74a48) |= word << (held & 31);
        held += 0x10;
    }
    held -= count;
    global<u32>(0x00aa67f8) = held;
    const u32 accumulator = global<u32>(0x00f74a48);
    const u32 result = ((1u << (count & 31)) - 1u) & accumulator;
    global<u32>(0x00f74a48) = accumulator >> (count & 31);
    return result;
}
}
