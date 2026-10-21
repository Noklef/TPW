#include "tgqvideo.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// value * factor, 16.16, rounded by the product's bit 15 (IMUL; SHL EDX,16; SHR EAX,16; ADC).
u32 fixmul(s32 value, s32 factor) {
    const int64_t product = (int64_t)value * factor;
    const u32 low = (u32)product, high = (u32)((uint64_t)product >> 32);
    return (high << 16) + (low >> 16) + (low >> 15 & 1);
}

s32 bit(u32 n) { return (s32)(1u << (n & 31)); }

// The coefficient decoder's bit reader (TgqVideo_DecodeCoefficients), in registers while it runs.
struct CoefficientReader {
    const u8 *source;
    u32 bits, count;

    // 3 bits dropped, then a byte read in at count when fewer than 8 are left (shl's count mod 32; the compare signed).
    void Drop3() {
        bits >>= 3;
        count -= 3;
        if ((s32)count < 8) {
            bits |= (u32)*source++ << (count & 31);
            count += 8;
        }
    }
    // 8 bits dropped and a byte read in at count - 8.
    void Next8() {
        bits >>= 8;
        bits |= (u32)*source++ << ((count - 8) & 31);
    }
};

u32 &coefficient(u32 index) { return global<u32>(0x00fb7600 + index * 4); }
u32 dequantiser(u32 index) { return global<u32>(0x00fb7820 + index * 4); }
u32 zigzag(s32 at) { return global<u32>(0x00fbb5ac + (u32)at * 4); }

// A channel rounded to bits (TgqVideo_PackColour).
s32 rounded(s32 v, u32 bits) {
    const s32 unit = bit(8 - bits);
    if (v < 0) {
        const s32 q = (s32)((u32)v - (u32)bit(7 - bits)) / unit;
        const s32 low = (s32)(1u - (u32)bit(bits));
        return q < low ? low : q;
    }
    const s32 q = (s32)((u32)bit(7 - bits) + (u32)v) / unit;
    const s32 high = (s32)((u32)bit(bits) - 1u);
    return q > high ? high : q;
}

// A table's entry for a value (its low byte, signed): value * [0xfb7820] as 16.16 rounded by bit 15, then >> 1 & 0x7f.
u32 entry(u32 table, s32 value) {
    const int64_t product = (int64_t)(s8)value * (s32)global<u32>(0x00fb7820);
    const u32 low = (u32)product, high = (u32)((uint64_t)product >> 32);
    const u32 scaled = (high << 16) + (low >> 16) + (low >> 15 & 1);
    return global<u32>(table + ((scaled >> 1) & 0x7f) * 4);
}

void fill16(u8 *at, u32 colour, u32 step) {
    const u32 pair = (colour & 0xffff) << 16 | (colour & 0xffff);
    for (s32 row = 0; row < 8; ++row, at += step)
        for (s32 k = 0; k < 4; ++k) reinterpret_cast<u32 *>(at)[k] = pair;
}

void fill32(u8 *at, u32 colour, u32 step) {
    for (s32 row = 0; row < 8; ++row, at += step)
        for (s32 k = 0; k < 8; ++k) reinterpret_cast<u32 *>(at)[k] = colour;
}

}  // namespace

extern "C" {

u32 __fastcall TgqVideo_PackColour(s32 r, s32 g, s32 b, u32, u32 red_bits, u32 green_bits, u32 blue_bits, u32 red_shift,
                                   u32 green_shift, u32 blue_shift) {
    s32 rt = r / bit(8 - red_bits), rr = rounded(r, red_bits);
    s32 gt = g / bit(8 - green_bits), gr = rounded(g, green_bits);
    s32 bt = b / bit(8 - blue_bits);
    const s32 br = rounded(b, blue_bits);
    if (br < 0) --gr;
    if (gr < 0) --rr;
    if (rr < 0) --bt;
    if (bt < 0) --gt;
    if (gt < 0) --rt;
    const u32 rm = (u32)bit(red_bits) - 1, gm = (u32)bit(green_bits) - 1, bm = (u32)bit(blue_bits) - 1;
    u32 pixel = ((u32)bt & bm) << ((blue_shift + 0x10) & 31) | ((u32)rt & rm) << ((red_shift + 0x10) & 31) |
                ((u32)gt & gm) << ((green_shift + 0x10) & 31) | ((u32)br & bm) << (blue_shift & 31) | ((u32)gr & gm) << (green_shift & 31) |
                ((u32)rr & rm) << (red_shift & 31);
    if (blue_bits + green_bits + red_bits != 0x10) pixel |= 0x80008000;
    return pixel;
}

void __fastcall TgqVideo_BuildTables(u32 unused, u32 red_bits, u32 green_bits, u32 blue_bits, u32 red_shift, u32 green_shift,
                                     u32 blue_shift) {
    for (u32 i = 0; i < 0x80; ++i) {
        const s32 v = (s8)(u8)(i * 2 + 1) + 0x80;
        global<u32>(0x00fb7930 + i * 4) = TgqVideo_PackColour(v, v, v, unused, red_bits, green_bits, blue_bits, red_shift, green_shift, blue_shift);
    }
    for (u32 i = 0; i < 0x80; ++i) {
        const s32 c = (s8)(u8)(i * 2 + 1);
        const s32 g = (s32)fixmul(c, -0x5816), b = (s32)fixmul(c, 0x1c5a1);
        global<u32>(0x00fb7d30 + i * 4) = TgqVideo_PackColour(0, g, b, 0, red_bits, green_bits, blue_bits, red_shift, green_shift, blue_shift);
    }
    for (u32 i = 0; i < 0x80; ++i) {
        const s32 c = (s8)(u8)(i * 2 + 1);
        const s32 r = (s32)fixmul(c, 0x166e9), g = (s32)fixmul(c, -0xb6cf);
        global<u32>(0x00fb7b30 + i * 4) = TgqVideo_PackColour(r, g, 0, 0, red_bits, green_bits, blue_bits, red_shift, green_shift, blue_shift);
    }
}

void __cdecl TgqVideo_SetQuality(s32 quality) {
    const u32 d = 100u - (u32)quality;
    const s32 base = (s32)(d * 22) / 100 + 8;
    const u32 step = (u32)((s32)(d * 50) / 100 + 10 - base);
    u32 row_start = 0;
    for (u32 i = 0; i < 0x40; row_start += step) {
        u32 at = row_start;
        for (s32 k = 0; k < 8; ++k, ++i, at += step) {
            const u32 scale = (u32)((s32)at / 14 + base) << 16;
            global<u32>(0x00fb7820 + i * 4) = fixmul(global<s32>(0x00705c88 + i * 4), (s32)scale);
        }
    }
}

void __cdecl TgqVideo_PixelRow16(const u32 *luma, u16 *out, const u32 *chroma) {
    volatile const u32 *const dropped = reinterpret_cast<volatile const u32 *>(out);
    (void)dropped[0];
    for (int k = 0; k < 8; ++k) {
        if (k == 4) (void)dropped[8];
        const u32 y0 = luma[2 * k], y1 = luma[2 * k + 1], u = chroma[k], v = chroma[k + 0x40];
        const u32 colour = global<u32>(0x00fb7d30 + (u >> 17 & 0x7f) * 4) + global<u32>(0x00fb7b30 + (v >> 17 & 0x7f) * 4);
        out[2 * k] = (u16)(global<u32>(0x00fb7930 + (y0 >> 17 & 0x7f) * 4) + colour);
        out[2 * k + 1] = (u16)(global<u32>(0x00fb7930 + (y1 >> 17 & 0x7f) * 4) + colour);
    }
}

s32 __cdecl TgqVideo_DecodeCoefficients() {
    CoefficientReader in = {global<const u8 *>(0x00fb7920), global<u32>(0x00fb7928), global<u32>(0x00fb7924)};
    coefficient(0) = fixmul((s32)(in.bits << 24) >> 8, (s32)dequantiser(0));
    in.Next8();
    s32 at = 0x3f;
    do {
        u32 index = zigzag(at);
        const u32 target = global<u32>(0x00fbbaac + (in.bits & 7) * 4);
        bool wide = true;
        switch (target) {
        case 0x00672ede:
            coefficient(index) = 0;
            wide = false;
            break;
        case 0x00672ebc:
            coefficient(index) = dequantiser(index);
            wide = false;
            break;
        case 0x00672ecc:
            coefficient(index) = 0u - dequantiser(index);
            wide = false;
            break;
        case 0x00672eeb:
            coefficient(index) = 0;
            index = zigzag(at - 1);
            --at;
            coefficient(index) = 0;
            wide = false;
            break;
        case 0x00672f36:
            coefficient(index) = 0;
            index = zigzag(at - 1);
            --at;
            [[fallthrough]];
        case 0x00672f49: {
            u32 pairs = (in.bits & 0xff) >> 3;
            do {
                coefficient(index) = 0;
                index = zigzag(at - 1);
                coefficient(index) = 0;
                index = zigzag(at - 2);
                at -= 2;
            } while (--pairs != 0);
            ++at;
            break;
        }
        case 0x00672f7a: {
            const u32 code = in.bits & 0xfc;
            if (code == 0xfc) {
                in.Next8();
                coefficient(index) = fixmul((s32)(in.bits << 24) >> 8, (s32)dequantiser(index));
            } else {
                coefficient(index) = fixmul((s32)(code << 24) >> 10, (s32)dequantiser(index));
            }
            break;
        }
        default:  // not code (the exe's codes 5-7): the original jumps there and faults, sic
            reinterpret_cast<void(__cdecl *)()>((uintptr_t)target)();
            break;
        }
        if (wide) {
            in.Next8();
        } else {
            in.Drop3();
        }
    } while (--at > 0);
    global<const u8 *>(0x00fb7920) = in.source;
    global<u32>(0x00fb7928) = in.bits;
    global<u32>(0x00fb7924) = in.count;
    return at;
}

void __cdecl TgqVideo_FillBlocks16(s32 y0, s32 y1, s32 y2, s32 y3, s32 c0, s32 c1, u8 *out, u32 pitch) {
    const u32 chroma = entry(0x00fb7d30, c0) + entry(0x00fb7b30, c1);
    const u32 step = (pitch >> 1) * 2;
    fill16(out, entry(0x00fb7930, y0) + chroma, step);
    fill16(out + 0x10, entry(0x00fb7930, y1) + chroma, step);
    fill16(out + pitch * 8, entry(0x00fb7930, y2) + chroma, step);
    fill16(out + pitch * 8 + 0x10, entry(0x00fb7930, y3) + chroma, step);
}

void __cdecl TgqVideo_FillBlocks32(s32 y0, s32 y1, s32 y2, s32 y3, s32 c0, s32 c1, u8 *out, u32 pitch) {
    const u32 chroma = entry(0x00fb7d30, c0) + entry(0x00fb7b30, c1);
    const u32 step = (pitch >> 2) * 4;
    fill32(out, entry(0x00fb7930, y0) + chroma, step);
    fill32(out + 0x20, entry(0x00fb7930, y1) + chroma, step);
    fill32(out + pitch * 8, entry(0x00fb7930, y2) + chroma, step);
    fill32(out + pitch * 8 + 0x20, entry(0x00fb7930, y3) + chroma, step);
}
}
