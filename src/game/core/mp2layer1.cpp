#include "mp2layer1.h"

#include <cstdint>
#include <cstring>
#include <mmintrin.h>

#include "../math/x87.h"
#include "mp2synth.h"

namespace {

constexpr u32 kMasks = 0x711700;          // u32[]: (1 << k) - 1 for k < 24
constexpr u32 kOffsets = 0x711860;        // s32[16]: -(2^n - 1)
constexpr u32 kFactors = 0x70ce08;        // float[][64]: muls[k][scale]
constexpr u32 kFactorsFixed = 0x70e908;   // s32[][64]: the same in fixed point (12 fraction bits)
constexpr u32 kFixedPoint = 0xfb2704;     // (a dword: nonzero, decode in fixed point with the MMX synthesis)

template <typename T> T Get(u32 address) {
    T v;
    std::memcpy(&v, reinterpret_cast<const void *>(static_cast<uintptr_t>(address)), sizeof v);
    return v;
}
template <typename T> void Put(u32 address, T v) { std::memcpy(reinterpret_cast<void *>(static_cast<uintptr_t>(address)), &v, sizeof v); }
u32 Addr(const void *p) { return static_cast<u32>(reinterpret_cast<uintptr_t>(p)); }

// The next k bits of the frame, the position moved on.
u32 ReadBits(u32 decoder, s32 k) {
    const u32 pos = Get<u32>(decoder + 0x242c);
    const u32 raw = Get<u32>(Get<u32>(decoder + 0x2428) + static_cast<u32>(static_cast<s32>(pos) >> 3));
    const u32 word = (raw >> 24) | ((raw >> 8) & 0xff00u) | ((raw << 8) & 0xff0000u) | (raw << 24);
    const u32 bits = (word >> ((0u - ((pos & 7) + static_cast<u32>(k))) & 31)) & Get<u32>(kMasks + 4u * static_cast<u32>(k));
    Put<u32>(decoder + 0x242c, pos + static_cast<u32>(k));
    return bits;
}

// A float sample: (sample - 2^n + 1) times the factor (fild, fmul m32, fstp).
float FloatSample(u32 sample, u32 n, u32 scale) {
    const s32 v = static_cast<s32>(sample - (1u << n) + 1);
    return static_cast<float>(X87_MulIntDouble(v, X87_Through(Get<float>(kFactors + 4 * (((n << 6) + scale) + 64)))));
}

// A fixed-point sample: its bits read, offset, times the factor in 32 bits, shifted right 12 (0 when unallocated).
s16 FixedSample(u32 decoder, s32 n, u32 scale) {
    if (!n) return 0;
    const u32 bits = ReadBits(decoder, n + 1);
    const u32 v = static_cast<u32>(Get<s32>(kOffsets + 4 * static_cast<u32>(n))) + bits;
    return static_cast<s16>(static_cast<s32>(v * Get<u32>(kFactorsFixed + 4 * (((static_cast<u32>(n) << 6) + scale) + 64))) >> 12);
}

s32 DecodeFloat(u32 decoder, u8 *decoder_, u32 pcm) {
    u32 alloc[64], scales[64], samples[64];
    float fraction[64];
    u32 k = 0;
    if (Get<s32>(decoder + 0x244c) == 2) {
        for (u32 i = 0; i < 32; ++i) {
            const u32 v = ReadBits(decoder, 8);
            alloc[2 * i] = v >> 4;
            alloc[2 * i + 1] = v & 0xf;
        }
        for (u32 j = 0; j < 64; ++j)
            if (alloc[j]) scales[k++] = ReadBits(decoder, 6);
    } else {
        for (u32 i = 0; i < 32; ++i) alloc[i] = ReadBits(decoder, 4);
        for (u32 j = 0; j < 32; ++j)
            if (alloc[j]) scales[k++] = ReadBits(decoder, 6);
    }
    for (u32 granule = 12; granule; --granule) {
        const u32 per = Get<s32>(decoder + 0x244c) == 2 ? 2 : 1;   // (two: the subbands' channels interleaved, channel 1's output at +0x80)
        k = 0;
        for (u32 j = 0; j < 32 * per; ++j)
            if (alloc[j]) samples[k++] = ReadBits(decoder, static_cast<s32>(alloc[j]) + 1);
        k = 0;
        for (u32 j = 0; j < 32 * per; ++j) {
            float &to = fraction[32 * (j % per) + j / per];
            if (!alloc[j]) {
                to = 0;
                continue;
            }
            to = FloatSample(samples[k], alloc[j], scales[k]);
            ++k;
        }
        Mp2Decoder_Synth(decoder_, nullptr, reinterpret_cast<s16 *>(static_cast<uintptr_t>(pcm)), fraction);
        pcm += 2 * Get<u32>(decoder + 0x2450);
    }
    return 1;
}

s32 DecodeFixed(u32 decoder, u8 *decoder_, u32 pcm) {
    s8 alloc[64];
    u32 scale[64];   // (the subband's channel 0 at i, channel 1 at 32 + i; mono: i)
    s16 fraction[12 * 64];
    const u32 pos = Get<u32>(decoder + 0x242c);
    const u32 at = Get<u32>(decoder + 0x2428) + static_cast<u32>(static_cast<s32>(pos) >> 3);
    if (Get<s32>(decoder + 0x244c) == 2) {
        Put<u32>(decoder + 0x242c, pos + 0x100);
        for (u32 i = 0; i < 32; ++i) {
            const u8 b = Get<u8>(at + i);
            alloc[2 * i] = static_cast<s8>(b >> 4), alloc[2 * i + 1] = static_cast<s8>(b & 0xf);
            if (alloc[2 * i]) scale[i] = ReadBits(decoder, 6);
            if (alloc[2 * i + 1]) scale[32 + i] = ReadBits(decoder, 6);
        }
        for (u32 g = 0; g < 12; ++g)
            for (u32 i = 0; i < 32; ++i)
                for (u32 c = 0; c < 2; ++c) fraction[64 * g + 32 * c + i] = FixedSample(decoder, alloc[2 * i + c], scale[32 * c + i]);
        for (u32 g = 0; g < 12; ++g) {
            Mp2Decoder_SynthFixed(decoder_, nullptr, reinterpret_cast<s16 *>(static_cast<uintptr_t>(pcm)), fraction + 64 * g);
            pcm += 2 * Get<u32>(decoder + 0x2450);
        }
    } else {
        Put<u32>(decoder + 0x242c, pos + 0x80);
        for (u32 i = 0; i < 32; i += 2) {
            const u8 b = Get<u8>(at + (i >> 1));
            alloc[i] = static_cast<s8>(b >> 4), alloc[i + 1] = static_cast<s8>(b & 0xf);
            if (alloc[i]) scale[i] = ReadBits(decoder, 6);
            if (alloc[i + 1]) scale[i + 1] = ReadBits(decoder, 6);
        }
        for (u32 g = 0; g < 12; ++g)
            for (u32 i = 0; i < 32; ++i) fraction[32 * g + i] = FixedSample(decoder, alloc[i], scale[i]);
        for (u32 g = 0; g < 12; ++g) {
            Mp2Decoder_SynthFixed(decoder_, nullptr, reinterpret_cast<s16 *>(static_cast<uintptr_t>(pcm)), fraction + 32 * g);
            pcm += 2 * Get<u32>(decoder + 0x2450);
        }
    }
    _mm_empty();
    return 1;
}

}  // namespace

extern "C" {

s32 __fastcall Mp2Decoder_DecodeLayer1(u8 *decoder_, void *, s16 *pcm) {
    const u32 decoder = Addr(decoder_);
    return Get<u32>(kFixedPoint) ? DecodeFixed(decoder, decoder_, Addr(pcm)) : DecodeFloat(decoder, decoder_, Addr(pcm));
}

}  // extern "C"
