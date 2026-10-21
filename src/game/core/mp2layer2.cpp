#include "mp2layer2.h"

#include <cstdint>
#include <cstring>
#include <mmintrin.h>

#include "../math/x87.h"
#include "mp2synth.h"

namespace {

constexpr u32 kMasks = 0x711700;          // u32[]: (1 << k) - 1 for k < 24
constexpr u32 kGroups = 0x782e00;         // the grouping tables' pointers by d (3, 5 and 9 levels): three bytes per code
constexpr u32 kFactors = 0x70ce08;        // float[][64]: muls[k][scale]
constexpr u32 kFactorsFixed = 0x70e908;   // s32[][64]: the same in fixed point (12 fraction bits)
constexpr u32 kTableChoice = 0x711570;    // u32[]: the allocation table (0-4) by bitrate, channels and sampling rate
constexpr u32 kSubbands = 0x711670;       // s32[5]: each table's sblimit
constexpr u32 kTables = 0x782e28;         // the 5 allocation tables' addresses
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

// The factor table's entry for a bit count (or grouped code) and a scale factor.
u32 Factor(u32 table, u32 row, s32 scale) { return table + 4u * ((row << 6) + static_cast<u32>(scale)); }
s16 Shift12(u32 product) { return static_cast<s16>(static_cast<s32>(product) >> 12); }

// The fixed-point form's channel of a subband coded per channel: its granules at at, at + 0x80, at + 0x100; the scale index moves on 3 when
// the allocation is not 0.
void FixedChannel(u32 decoder, u32 at, s32 ba, u32 table, u32 scale, s32 &x) {
    if (ba == 0) {
        for (u32 g = 0; g < 3; ++g) Put<s16>(at + 0x80 * g, 0);
        return;
    }
    const u32 entry = table + 4u * static_cast<u32>(ba);
    const s32 k = Get<s16>(entry), d = Get<s16>(entry + 2);
    const s32 s = Get<s8>(scale + static_cast<u32>(x));
    if (d < 0) {
        const u32 factor = Get<u32>(Factor(kFactorsFixed, static_cast<u32>(k), s));
        for (u32 g = 0; g < 3; ++g) Put<s16>(at + 0x80 * g, Shift12((ReadBits(decoder, k) + static_cast<u32>(d)) * factor));
    } else {
        const u32 group = Get<u32>(kGroups + 4u * static_cast<u32>(d)) + 3u * ReadBits(decoder, k);
        for (u32 g = 0; g < 3; ++g) Put<s16>(at + 0x80 * g, Shift12(Get<u32>(Factor(kFactorsFixed, Get<u8>(group + g), s))));
    }
    x += 3;
}

// The float form's.
void FloatChannel(u32 decoder, u32 at, s32 ba, u32 table, u32 scale, s32 &x) {
    if (ba == 0) {
        for (u32 g = 0; g < 3; ++g) Put<u32>(at + 0x100 * g, 0);
        return;
    }
    const u32 entry = table + 4u * static_cast<u32>(ba);
    const s32 k = Get<s16>(entry), d = Get<s16>(entry + 2);
    if (d < 0) {
        const float factor = X87_Through(Get<float>(Factor(kFactors, static_cast<u32>(k), Get<s8>(scale + static_cast<u32>(x)))));
        for (u32 g = 0; g < 3; ++g) {
            const s32 v = static_cast<s32>(ReadBits(decoder, k) + static_cast<u32>(d));
            Put<float>(at + 0x100 * g, static_cast<float>(X87_MulIntDouble(v, factor)));
        }
    } else {
        const s32 s = Get<s8>(scale + static_cast<u32>(x));
        const u32 group = Get<u32>(kGroups + 4u * static_cast<u32>(d)) + 3u * ReadBits(decoder, k);
        for (u32 g = 0; g < 3; ++g) Put<u32>(at + 0x100 * g, Get<u32>(Factor(kFactors, Get<u8>(group + g), s)));
    }
    x += 3;
}

}  // namespace

extern "C" {

void __fastcall Mp2Decoder_Dequantize(u8 *decoder_, void *, const s8 *alloc_bits, float *out, const s8 *scale_, s32 scale_at, const s16 *table_) {
    const u32 decoder = Addr(decoder_), base = Addr(out), scale = Addr(scale_);
    u32 bits = Addr(alloc_bits), table = Addr(table_);
    s32 x = scale_at;
    const s32 jsbound = Get<s32>(decoder + 0x241c), sblimit = Get<s32>(decoder + 0x2418);
    for (s32 i = 0; i < jsbound; ++i) {
        const s32 step = Get<s16>(table);
        for (s32 c = 0; c < Get<s32>(decoder + 0x244c); ++c) {
            const s32 ba = Get<s8>(bits++);
            FloatChannel(decoder, base + 4u * static_cast<u32>(i) + 0x80u * static_cast<u32>(c), ba, table, scale, x);
        }
        table += 4u * Get<u32>(kMasks + 4u * static_cast<u32>(step)) + 4;
    }
    for (s32 i = jsbound; i < sblimit; ++i) {   // joint stereo: channel 1's allocation for both
        const u32 at = base + 4u * static_cast<u32>(i);
        const s32 step = Get<s16>(table);
        bits += 2;
        const s32 ba = Get<s8>(bits - 1);
        if (ba == 0) {
            for (u32 g = 0; g < 3; ++g) {
                Put<u32>(at + 0x100 * g, 0);
                Put<u32>(at + 0x100 * g + 0x80, 0);
            }
        } else {
            const u32 entry = table + 4u * static_cast<u32>(ba);
            const s32 k = Get<s16>(entry), d = Get<s16>(entry + 2);
            if (d < 0) {
                const float factor1 = X87_Through(Get<float>(Factor(kFactors, static_cast<u32>(k), Get<s8>(scale + static_cast<u32>(x) + 3))));
                for (u32 g = 0; g < 3; ++g) {
                    const s32 v = static_cast<s32>(ReadBits(decoder, k) + static_cast<u32>(d));
                    Put<float>(at + 0x100 * g, static_cast<float>(v));
                    Put<float>(at + 0x100 * g + 0x80, static_cast<float>(X87_MulIntDouble(v, factor1)));
                }
                const X87Reg factor0 = Get<float>(Factor(kFactors, static_cast<u32>(k), Get<s8>(scale + static_cast<u32>(x))));
                for (u32 g = 0; g < 3; ++g) Put<float>(at + 0x100 * g, static_cast<float>(factor0 * Get<float>(at + 0x100 * g)));
            } else {
                const s32 s0 = Get<s8>(scale + static_cast<u32>(x)), s1 = Get<s8>(scale + static_cast<u32>(x) + 3);
                const u32 group = Get<u32>(kGroups + 4u * static_cast<u32>(d)) + 3u * ReadBits(decoder, k);
                for (u32 g = 0; g < 3; ++g) {
                    const u8 code = Get<u8>(group + g);
                    Put<u32>(at + 0x100 * g, Get<u32>(Factor(kFactors, code, s0)));
                    Put<u32>(at + 0x100 * g + 0x80, Get<u32>(Factor(kFactors, code, s1)));
                }
            }
            x += 6;
        }
        table += 4u << (step & 31);
    }
    const u32 channels = Get<s32>(decoder + 0x244c) == 2 ? 2 : 1;
    for (s32 i = sblimit; i < 32; ++i)
        for (u32 g = 0; g < 3; ++g)
            for (u32 c = 0; c < channels; ++c) Put<u32>(base + 4u * static_cast<u32>(i) + 0x100 * g + 0x80 * c, 0);
}

void __fastcall Mp2Decoder_DequantizeFixed(u8 *decoder_, void *, const s8 *alloc_bits, s16 *out, const s8 *scale_, s32 scale_at, const s16 *table_) {
    const u32 decoder = Addr(decoder_), base = Addr(out), scale = Addr(scale_);
    u32 bits = Addr(alloc_bits), table = Addr(table_);
    s32 x = scale_at;
    for (s32 i = 0; i < Get<s32>(decoder + 0x241c); ++i) {
        const s32 step = Get<s16>(table);
        const u32 at = base + 2u * static_cast<u32>(i);
        FixedChannel(decoder, at, Get<s8>(bits++), table, scale, x);
        if (Get<s32>(decoder + 0x244c) == 2) FixedChannel(decoder, at + 0x40, Get<s8>(bits++), table, scale, x);
        table += 4u * Get<u32>(kMasks + 4u * static_cast<u32>(step)) + 4;
    }
    for (s32 i = Get<s32>(decoder + 0x241c); i < Get<s32>(decoder + 0x2418); ++i) {   // joint stereo: channel 1's allocation for both
        const u32 at = base + 2u * static_cast<u32>(i);
        const s32 step = Get<s16>(table);
        bits += 2;
        const s32 ba = Get<s8>(bits - 1);
        if (ba == 0) {
            for (u32 g = 0; g < 3; ++g) {
                Put<s16>(at + 0x80 * g, 0);
                Put<s16>(at + 0x80 * g + 0x40, 0);
            }
        } else {
            const u32 entry = table + 4u * static_cast<u32>(ba);
            const s32 k = Get<s16>(entry), d = Get<s16>(entry + 2);
            if (d < 0) {
                const u32 factor1 = Get<u32>(Factor(kFactorsFixed, static_cast<u32>(k), Get<s8>(scale + static_cast<u32>(x) + 3)));
                for (u32 g = 0; g < 3; ++g) {
                    const s16 v = static_cast<s16>(ReadBits(decoder, k) + static_cast<u32>(d));
                    Put<s16>(at + 0x80 * g, v);
                    Put<s16>(at + 0x80 * g + 0x40, Shift12(factor1 * static_cast<u32>(static_cast<s32>(v))));
                }
                const u32 factor0 = Get<u32>(Factor(kFactorsFixed, static_cast<u32>(k), Get<s8>(scale + static_cast<u32>(x))));
                for (u32 g = 0; g < 3; ++g)
                    Put<s16>(at + 0x80 * g, Shift12(factor0 * static_cast<u32>(static_cast<s32>(Get<s16>(at + 0x80 * g)))));
            } else {
                const s32 s0 = Get<s8>(scale + static_cast<u32>(x)), s1 = Get<s8>(scale + static_cast<u32>(x) + 3);
                const u32 group = Get<u32>(kGroups + 4u * static_cast<u32>(d)) + 3u * ReadBits(decoder, k);
                for (u32 g = 0; g < 3; ++g) {
                    const u8 code = Get<u8>(group + g);
                    Put<s16>(at + 0x80 * g, Shift12(Get<u32>(Factor(kFactorsFixed, code, s0))));
                    Put<s16>(at + 0x80 * g + 0x40, Shift12(Get<u32>(Factor(kFactorsFixed, code, s1))));
                }
            }
            x += 6;
        }
        table += 4u * Get<u32>(kMasks + 4u * static_cast<u32>(step)) + 4;
    }
    // (unsigned: a negative sblimit zeroes nothing, as the original's jump table)
    for (u32 i = Get<u32>(decoder + 0x2418); i < 32; ++i)
        for (u32 g = 0; g < 3; ++g) Put<s16>(base + 2 * i + 0x80 * g, 0);
    if (Get<s32>(decoder + 0x244c) == 2)
        for (u32 i = Get<u32>(decoder + 0x2418); i < 32; ++i)
            for (u32 g = 0; g < 3; ++g) Put<s16>(base + 2 * i + 0x80 * g + 0x40, 0);
}

s32 __fastcall Mp2Decoder_DecodeLayer2(u8 *decoder_, void *, s16 *pcm) {
    const u32 decoder = Addr(decoder_);
    s8 bitAlloc[64];
    u8 scfsi[64];
    s8 scale[192];
    float fraction[3 * 2 * 32];
    s16 fixed[3 * 2 * 32];
    const u32 choice = ((2u * Get<u8>(decoder + 0x2446) - Get<u32>(decoder + 0x244c)) << 4) + Get<u8>(decoder + 0x2445);
    const u32 table = Get<u8>(decoder + 0x243f) ? 4 : Get<u32>(kTableChoice + 4 * choice);
    s32 jsbound = Get<s32>(kSubbands + 4 * table);
    Put<s32>(decoder + 0x2418, jsbound);
    const u32 table0 = Get<u32>(kTables + 4 * table);
    if (Get<u8>(decoder + 0x2448) == 1) jsbound = 4 * Get<u8>(decoder + 0x2449) + 4;   // joint stereo: the mode extension's bound
    Put<s32>(decoder + 0x241c, jsbound);
    const s32 sblimit = Get<s32>(decoder + 0x2418), channels = Get<s32>(decoder + 0x244c);
    const u32 coded = static_cast<u32>(sblimit * channels);
    u32 alloc = table0, ba = Addr(bitAlloc), sf = Addr(scfsi);
    // The allocations (two per subband; the joint subbands read one for both), then the scale factor selection of each allocated one.
    if (channels == 2) {
        for (u32 n = static_cast<u32>(jsbound); n; --n) {
            const s32 step = Get<s16>(alloc);
            Put<u8>(ba++, static_cast<u8>(ReadBits(decoder, step)));
            Put<u8>(ba++, static_cast<u8>(ReadBits(decoder, step)));
            alloc += 4u << (step & 31);
        }
        for (u32 n = static_cast<u32>(sblimit - jsbound); n; --n) {
            const s32 step = Get<s16>(alloc);
            const u8 v = static_cast<u8>(ReadBits(decoder, step));
            Put<u8>(ba++, v);
            Put<u8>(ba++, v);
            alloc += 4u << (step & 31);
        }
        u32 p = Addr(bitAlloc);
        for (u32 n = coded; n; --n)
            if (Get<u8>(p++)) Put<u8>(sf++, static_cast<u8>(ReadBits(decoder, 2)));
    } else {
        for (u32 n = static_cast<u32>(sblimit); n; --n) {
            const s32 step = Get<s16>(alloc);
            Put<u8>(ba++, static_cast<u8>(ReadBits(decoder, step)));
            alloc += 4u << (step & 31);
        }
        u32 p = Addr(bitAlloc);
        for (u32 n = Get<u32>(decoder + 0x2418); n; --n)
            if (Get<u8>(p++)) Put<u8>(sf++, static_cast<u8>(ReadBits(decoder, 2)));
    }
    // The scale factors: three per allocated subband and channel, as many read as the selection says (6 bits each).
    u32 p = Addr(bitAlloc), at = Addr(scale);
    sf = Addr(scfsi);
    for (u32 n = coded; n; --n) {
        if (!Get<u8>(p++)) continue;
        u8 a, b, c;
        switch (Get<s8>(sf++)) {
        case 0: {
            const u32 v = ReadBits(decoder, 18);
            a = (v >> 12) & 0x3f, b = (v >> 6) & 0x3f, c = v & 0x3f;
            break;
        }
        case 1: {
            const u32 v = ReadBits(decoder, 12);
            a = b = (v >> 6) & 0x3f, c = v & 0x3f;
            break;
        }
        case 2:
            a = b = c = static_cast<u8>(ReadBits(decoder, 6));
            break;
        default: {
            const u32 v = ReadBits(decoder, 12);
            a = (v >> 6) & 0x3f, b = c = v & 0x3f;
            break;
        }
        }
        Put<u8>(at++, a);
        Put<u8>(at++, b);
        Put<u8>(at++, c);
    }
    // Twelve groups of three granules: dequantised, then each granule through the synthesis (the output moving on +0x2450 samples).
    const s16 *tab = reinterpret_cast<const s16 *>(static_cast<uintptr_t>(table0));
    u32 out = Addr(pcm);
    for (s32 group = 0; group < 12; ++group) {
        const bool pcmGroup = out != 0;   // (none: the synthesis only runs the DCTs, the output stays none)
        if (!Get<u32>(kFixedPoint)) {
            Mp2Decoder_Dequantize(decoder_, nullptr, bitAlloc, fraction, scale, group >> 2, tab);
            for (u32 g = 0; g < 3; ++g) {
                Mp2Decoder_Synth(decoder_, nullptr, reinterpret_cast<s16 *>(static_cast<uintptr_t>(out)), fraction + 0x40 * g);
                if (pcmGroup) out += 2 * Get<u32>(decoder + 0x2450);
            }
        } else {
            Mp2Decoder_DequantizeFixed(decoder_, nullptr, bitAlloc, fixed, scale, group >> 2, tab);
            for (u32 g = 0; g < 3; ++g) {
                Mp2Decoder_SynthFixed(decoder_, nullptr, reinterpret_cast<s16 *>(static_cast<uintptr_t>(out)), fixed + 0x40 * g);
                if (pcmGroup) out += 2 * Get<u32>(decoder + 0x2450);
            }
        }
    }
    if (Get<u32>(kFixedPoint)) _mm_empty();   // (the MMX synthesis's state; nothing here leaves any)
    return 1;
}

}  // extern "C"
