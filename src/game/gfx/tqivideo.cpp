#include "tqivideo.h"

#include <cstdint>

#include "../math/x87.h"
#include "movieidct.h"

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

}  // namespace

extern "C" {

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
