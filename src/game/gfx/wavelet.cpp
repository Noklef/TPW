#include "wavelet.h"

#include <cstdint>
#include "../core/vcall.h"
#include "../core/gameheap.h"

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

u32 __fastcall WaveletCodec_Decode(u8 *codec, void *, s32 version, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j) {
    u8 *const base = codec - 0x54;
    if (version == 0x12)
        return vcall<u32>(base, 0, a, b, c, d, e, f, g, h, i, j);
    if (version == 0x13)
        return vcall<u32>(base, 8, a, b, c, d, e, f, g, h, i, j);
    return (u32)(version - 0x13);  // (what the dispatch left in EAX)
}

u32 __fastcall WaveletCodec_DecodeScaled(u8 *codec, void *, s32 version, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j, u32 k, u32 l, u32 m) {
    if (version > 0x12)
        return vcall<u32>(codec - 0x54, 4, a, b, c, d, e, f, g, h, i, j, k, l, m);
    return 0;
}

void __fastcall WaveletCodec_VBaseDtor(u8 *base) {
    // The SEH frame is left out.
    at<u32>(base, -0x54) = 0x7026d8;
    const u32 offset = at<u32>(at<u8 *>(base, -0x50), 4);
    at<u32>(base, offset - 0x50) = 0x7026b8;
    at<u32>(base, offset - 0x54) = offset - 0x50;
    for (u32 i = 1; i <= 5; i++) {
        if (void *const block = at<void *>(base, -0x10 * i + 8))
            GameHeap_Free(block);
    }
}
}
