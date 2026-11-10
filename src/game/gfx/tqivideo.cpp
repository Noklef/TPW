#include "tqivideo.h"

#include <cstdint>

#include "../math/x87.h"
#include "movieidct.h"
#include "tgqvideo.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// value * factor, 16.16, rounded by the product's bit 15 (IMUL; SHL EDX,16; SHR EAX,16; ADC).
s32 fixmul(s32 value, s32 factor) {
    const int64_t product = (int64_t)value * factor;
    const u32 low = (u32)product, high = (u32)((uint64_t)product >> 32);
    return (s32)((high << 16) + (low >> 16) + (low >> 15 & 1));
}

s32 ftol(X87Reg x) { return (s32)(long long)x.v; }   // __ftol (0x67a830): truncated, its low dword

s32 idct_scale(u32 i) { return global<s32>(0x00705c88 + i * 4); }

// The 32 bits at the bit position (SHLD by its low 5 bits; the second dword read whatever the count).
u32 peek(const u32 *bits, u32 position) {
    const u32 first = bits[position >> 5], second = bits[(position >> 5) + 1], count = position & 31;
    return count ? first << count | second >> (32 - count) : first;
}

// The entry of the 17 bits code (nonzero, at least 0x20): by its first bit set.
u32 coefficientEntry(u32 code) {
    if (code >= 0x8000) return global<u32>(0x00fbb69c + (code >> 13) * 4);
    if (code >= 0x800) return global<u32>(0x00fbb6cc + (code >> 9) * 4);
    if (code >= 0x400) return global<u32>(0x00fbb7ac + (code >> 7) * 4);
    if (code >= 0x200) return global<u32>(0x00fbb7ac + (code >> 5) * 4);
    if (code >= 0x100) return global<u32>(0x00fbb7ec + (code >> 4) * 4);
    if (code >= 0x80) return global<u32>(0x00fbb82c + (code >> 3) * 4);
    if (code >= 0x40) return global<u32>(0x00fbb86c + (code >> 2) * 4);
    return global<u32>(0x00fbb8ac + (code >> 1) * 4);
}

// A DC difference: `bits` top bits of the stream index `table`'s bytes (the code's length, the number of bits that follow).
s32 readDifference(const u32 *bits, u32 *position, u32 table, u32 indexBits) {
    const u32 entry = global<u8>(table + (peek(bits, *position) >> (32 - indexBits)));
    const u32 n = entry >> 4;
    *position += entry & 0xf;
    if (!n) return 0;
    u32 value = peek(bits, *position) >> (32 - n);
    *position += n;
    if (!(value >> (n - 1) & 1)) value -= (1u << n) - 1;
    return static_cast<s32>(value);
}

// The 16 rows of a macroblock through a row writer.
template <typename Out, typename Row> void macroblock(Out *out, s32 pitch, u8 *state, Row row) {
    u32 blocks[0x200];
    u32 &position = *reinterpret_cast<u32 *>(state + 0x204);
    position = TqiVideo_ReadBlocks(state, *reinterpret_cast<const u32 **>(state + 0x200), position, blocks);
    for (u32 j = 0; j < 16; ++j) row(blocks + 0x10 * j, blocks + 0x100 + 8 * (j / 2), reinterpret_cast<Out *>(reinterpret_cast<u8 *>(out) + static_cast<s32>(j) * pitch));
}

u32 yuy2Part(u32 index) { return global<u32>(0x00fb8730 + index * 8); }

// The pixel of a sum: its bits 23-31, 11-19 and 0-8 through their tables.
u32 rgbOf(u32 sum) { return global<u32>(0x00fba730 + (sum >> 0x17) * 4) | global<u32>(0x00fbabf8 + ((sum & 0xff800) >> 0xb) * 4) | global<u32>(0x00fbb010 + (sum & 0x1ff) * 4); }
u32 chromaSum(const u32 *chroma, u32 k) { return global<u32>(0x00fb9b30 + chroma[k] * 4) + global<u32>(0x00fba330 + chroma[k + 0x40] * 4); }
u32 lumaPart(u32 luma) { return global<u32>(0x00fb9330 + luma * 4); }

}  // namespace

extern "C" {

u32 __cdecl TqiVideo_ReadBlocks(u8 *state, const u32 *bits, u32 position, u32 *blocks) {
    static const struct { u32 offset, predictor, table, indexBits; s32 pitch; } kBlocks[6] = {
        {0, 0x100, 0xfbb92c, 7, 0x40}, {0x20, 0x100, 0xfbb92c, 7, 0x40}, {0x200, 0x100, 0xfbb92c, 7, 0x40}, {0x220, 0x100, 0xfbb92c, 7, 0x40},
        {0x400, 0x104, 0xfbb9ac, 8, 0x20}, {0x500, 0x108, 0xfbb9ac, 8, 0x20}};
    const s32 *const quant = reinterpret_cast<const s32 *>(state);
    for (const auto &block : kBlocks) {
        s32 &predictor = *reinterpret_cast<s32 *>(state + block.predictor);
        predictor = static_cast<s32>(static_cast<u32>(readDifference(bits, &position, block.table, block.indexBits)) + static_cast<u32>(predictor));
        position = TqiVideo_DecodeBlock(predictor, quant, bits, position, reinterpret_cast<u8 *>(blocks) + block.offset, block.pitch);
    }
    return position;
}

u32 __cdecl TqiVideo_DecodeBlock(s32 dc, const s32 *quant, const u32 *bits, u32 position, u8 *out, s32 pitch) {
    s32 block[64] = {};
    block[0] = (s32)((u32)dc * (u32)quant[0]);
    u8 index = 0x3f;
    for (;;) {
        const u32 code = peek(bits, position) >> 15;
        if (code < 0x20) break;
        const u32 entry = coefficientEntry(code), length = entry >> 16, run = entry >> 8 & 0xff;
        position += length;
        if (run == 0x41) break;
        s32 level;
        if (run > 0x41) {
            const u32 escape = peek(bits, position) >> 18;
            position += 14;
            index = (u8)(index - (escape >> 8) - 1);
            level = (s32)(escape & 0xff);
            if (level == 0 || level == 0x80) {
                const s32 next = (s32)(peek(bits, position) >> 24);
                position += 8;
                level = level ? next - 0x100 : next;
            } else if (level > 0x80) {
                level -= 0x100;
            }
        } else {
            index = (u8)(index - run);
            level = (s32)(entry & 0xff);
            ++position;
            if (code >> ((0x10 - length) & 31) & 1) level = -level;
        }
        const u32 place = global<u32>(0x00fbb5b0 + index * 4u);
        block[place] = (s32)((u32)level * (u32)quant[place]);
    }
    if (index == 0x3f) {
        const s32 value = block[0] >> 17;
        for (u32 y = 0; y < 8; ++y, out += pitch)
            for (u32 x = 0; x < 8; ++x) reinterpret_cast<s32 *>(out)[x] = value;
        return position;
    }
    s32 rows[8 * 9];
    for (u32 i = 0; i < 8; ++i) MovieIdct_Column(block + i * 8, rows + i);
    for (u32 i = 0; i < 8; ++i, out += pitch) MovieIdct_RowShift(rows + i * 9, reinterpret_cast<s32 *>(out));
    return position;
}

void __cdecl TqiVideo_SetQuality(s32 *out, s32 quality) {
    if (quality == 100) {
        for (u32 i = 0; i < 0x40; ++i) out[i] = fixmul(idct_scale(i), 0x80000);
    } else {
        const float scale = (float)(X87_SubR(X87Reg((double)quality), X87Reg(107.0)) * X87Reg(0.0625f));
        for (u32 i = 0; i < 0x40; ++i) {
            const s32 factor = ftol(X87Reg((double)global<s32>(0x00705d88 + i * 4)) * X87Reg(scale) * X87Reg(65536.0f));
            out[i] = fixmul(idct_scale(i), factor);
        }
    }
    out[0] = fixmul(idct_scale(0), 0x80000);
}
}

extern "C" {

void __cdecl TqiVideo_PixelRowYuy2(const u32 *luma, const u32 *chroma, u32 *out) {
    for (u32 k = 0; k < 8; ++k)
        out[k] = (yuy2Part(chroma[k + 0x40]) << 8 | yuy2Part(luma[2 * k + 1])) << 16 | yuy2Part(chroma[k]) << 8 | yuy2Part(luma[2 * k]);
}

void __cdecl TqiVideo_PixelRowRgb16(const u32 *luma, const u32 *chroma, u32 *out) {
    for (u32 k = 0; k < 8; ++k) {
        const u32 sum = chromaSum(chroma, k);
        out[k] = (rgbOf(lumaPart(luma[2 * k]) + sum) & 0xffff) | (rgbOf(lumaPart(luma[2 * k + 1]) + sum) & 0xffff0000);
    }
}

void __cdecl TqiVideo_PixelRowRgb32(const u32 *luma, const u32 *chroma, u32 *out) {
    for (u32 k = 0; k < 8; ++k) {
        const u32 sum = chromaSum(chroma, k);
        out[2 * k] = rgbOf(lumaPart(luma[2 * k]) + sum);
        out[2 * k + 1] = rgbOf(lumaPart(luma[2 * k + 1]) + sum);
    }
}

void __cdecl TqiVideo_DecodeMacroblockYuy2(u32 *out, s32 pitch, u8 *state) { macroblock(out, pitch, state, TqiVideo_PixelRowYuy2); }
void __cdecl TqiVideo_DecodeMacroblockRgb16(u32 *out, s32 pitch, u8 *state) { macroblock(out, pitch, state, TqiVideo_PixelRowRgb16); }
void __cdecl TqiVideo_DecodeMacroblockRgb32(u32 *out, s32 pitch, u8 *state) { macroblock(out, pitch, state, TqiVideo_PixelRowRgb32); }
void __cdecl TqiVideo_DecodeMacroblockTable16(u16 *out, s32 pitch, u8 *state) {
    macroblock(out, pitch, state, [](const u32 *luma, const u32 *chroma, u16 *row) { TgqVideo_PixelRow16(luma, row, chroma); });
}

void __cdecl TqiVideo_PixelRowBytes32(const u32 *luma, const u32 *chroma, u32 *out) {
    const auto mapped = [](u32 value) -> u32 { return global<u8>(0x00fb7f30 + (value >> 16 & 0xff)); };
    for (u32 k = 0; k < 8; ++k) {
        out[k] = mapped(luma[2 * k]) << 24 | mapped(luma[2 * k + 1]) << 16 | mapped(chroma[k]) << 8 | mapped(chroma[0x40 + k]);
    }
}

void __cdecl TqiVideo_BlockBytes32(const u32 *luma, const u32 *chroma, u32 *out, s32 pitch) {
    for (s32 j = 0; j < 16; ++j) {
        TqiVideo_PixelRowBytes32(luma + 0x10 * j, chroma + 8 * (j / 2), reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(out) + pitch * j));
    }
}
}
