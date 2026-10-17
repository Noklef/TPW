#include "wavelet.h"

#include <cstdint>
#include "../core/vcall.h"
#include "../core/gameheap.h"
#include "../math/x87.h"

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

void __fastcall WaveletCodec_UnpackBgr(u8 *codec, void *, u32 width, u32 height, u32 stride, const u8 *src) {
    if (height == 0)
        return;
    u32 row = 0;  // byte offset of the row in the planes (wraps as the original's does)
    do {
        u32 offset = row;
        for (u32 x = width; x != 0; --x) {
            *reinterpret_cast<float *>(at<u8 *>(codec, 0x2c) + offset) = (float)*src++;
            *reinterpret_cast<float *>(at<u8 *>(codec, 0x1c) + offset) = (float)*src++;
            *reinterpret_cast<float *>(at<u8 *>(codec, 0x0c) + offset) = (float)*src++;
            offset += 4;
        }
        row += stride * 4;
    } while (--height != 0);
}

void __fastcall WaveletCodec_RgbToYuv(u8 *codec, void *, u32 count) {
    for (u32 i = 0; i < count; ++i) {
        float *const bp = at<float *>(codec, 0x2c) + i;
        const X87Reg g = at<float *>(codec, 0x1c)[i];
        const X87Reg r = at<float *>(codec, 0x0c)[i];
        const X87Reg b = *bp;
        *bp = (float)(((g * 0.587f) - (b * -0.114f)) - (r * -0.299f));                // 0x702664, 0x702668, 0x70266c
        at<float *>(codec, 0x1c)[i] = (float)((b * 0.5f) - ((r * 0.168736f) - (g * -0.331264f)));  // 0x702670, 0x702674, 0x702678
        at<float *>(codec, 0x0c)[i] = (float)(((r * 0.5f) - (g * 0.418688f)) - (b * 0.081312f));   // 0x70267c, 0x702680
    }
}

void __fastcall WaveletCodec_ForwardD4Row(u8 *, void *, const float *src, float *dst, s32 n) {
    if (n <= 0)
        return;
    const u32 mask = (u32)(n - 1);
    float *high = dst + (n >> 1);
    const float *pair = src;  // s[2k], s[2k + 1]; s[2k + 2] and s[2k + 3] wrap around the row (n a power of two)
    s32 c = 3;
    do {
        const X87Reg h1s1 = X87Reg(global<float>(0x00c2720c)) * pair[1];
        const X87Reg h2s2 = X87Reg(global<float>(0x00c27210)) * src[(u32)(c - 1) & mask];
        const X87Reg h3s3 = X87Reg(global<float>(0x00c27214)) * src[(u32)c & mask];
        const X87Reg h0s0 = X87Reg(global<float>(0x00c27208)) * pair[0];
        *dst++ = (float)(((h1s1 + h2s2) + h3s3) + h0s0);
        const X87Reg h3s0 = X87Reg(global<float>(0x00c27214)) * pair[0];
        const X87Reg h2s1 = X87Reg(global<float>(0x00c27210)) * pair[1];
        const X87Reg h1s2 = X87Reg(global<float>(0x00c2720c)) * src[(u32)(c - 1) & mask];
        const X87Reg h0s3 = X87Reg(global<float>(0x00c27208)) * src[(u32)c & mask];
        *high++ = (float)(((h3s0 - h2s1) + h1s2) - h0s3);
        pair += 2;
        c += 2;
    } while (c - 3 < n);
}

void __fastcall WaveletCodec_HalvePlane(u8 *, void *, float *const *plane, u32 n) {
    const u32 half = n >> 1;
    if (half == 0)
        return;
    u32 top = 0, bottom = n * 4, out = 0;  // byte offsets (wrapping as the original's)
    for (u32 y = half; y != 0; --y) {
        for (u32 x = 0; x < half; ++x) {
            const u8 *const b = reinterpret_cast<const u8 *>(*plane);
            const X87Reg sum = ((X87Reg(at<float>(b, bottom + 8 * x)) + at<float>(b, bottom + 8 * x + 4)) + at<float>(b, top + 8 * x + 4)) +
                               at<float>(b, top + 8 * x);
            at<float>(b, out + 4 * x) = (float)(sum * 0.25f);  // 0x702684
        }
        top += n * 8;
        bottom += n * 8;
        out += half * 4;
    }
}
}
