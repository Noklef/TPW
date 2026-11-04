#include "crtfloat.h"

#include <cstdint>
#include <cstring>

namespace {

void zero_mantissa(u32 *mantissa) {
    mantissa[0] = 0;
    mantissa[1] = 0;
    mantissa[2] = 0;
}

// (The shifts take the count as x86 does: its low five bits.)
u32 shl(u32 value, s32 count) { return value << (count & 31); }
u32 shr(u32 value, s32 count) { return value >> (count & 31); }

}  // namespace

extern "C" {

u32 __cdecl CRT_abstract_cw(u32 word) {
    u32 out = word & 1 ? 0x10 : 0;
    if (word & 4) out |= 8;
    if (word & 8) out |= 4;
    if (word & 0x10) out |= 2;
    if (word & 0x20) out |= 1;
    if (word & 2) out |= 0x80000;
    const u32 rounding = word & 0xc00;
    if (rounding == 0x400) out |= 0x100;
    else if (rounding == 0x800) out |= 0x200;
    else if (rounding == 0xc00) out |= 0x300;
    const u32 precision = word & 0x300;
    if (!precision) out |= 0x20000;
    else if (precision == 0x200) out |= 0x10000;
    if (word & 0x1000) out |= 0x40000;
    return out;
}

u32 __cdecl CRT_hw_cw(u32 word) {
    u32 out = word & 0x10 ? 1 : 0;
    if (word & 8) out |= 4;
    if (word & 4) out |= 8;
    if (word & 2) out |= 0x10;
    if (word & 1) out |= 0x20;
    if (word & 0x80000) out |= 2;
    const u32 rounding = word & 0x300;
    if (rounding == 0x100) out |= 0x400;
    else if (rounding == 0x200) out |= 0x800;
    else if (rounding == 0x300) out |= 0xc00;
    const u32 precision = word & 0x30000;
    if (!precision) out |= 0x300;
    else if (precision == 0x10000) out |= 0x200;
    if (word & 0x40000) out |= 0x1000;
    return out;
}

s32 __cdecl CRT_addl(u32 a, u32 b, u32 *sum) {
    const u32 total = a + b;
    *sum = total;
    return total < a || total < b;
}

s32 __cdecl CRT_ZeroTail(const u32 *mantissa, s32 bit) {
    s32 word = bit / 32;
    if (mantissa[word] & ~shl(0xffffffffu, 31 - bit % 32)) return 0;
    for (++word; word < 3; ++word)
        if (mantissa[word]) return 0;
    return 1;
}

s32 __cdecl CRT_IncMan(u32 *mantissa, s32 bit) {
    s32 word = bit / 32;
    s32 carry = CRT_addl(mantissa[word], shl(1, 31 - bit % 32), &mantissa[word]);
    for (--word; word >= 0 && carry; --word) carry = CRT_addl(mantissa[word], 1, &mantissa[word]);
    return carry;
}

s32 __cdecl CRT_RoundMan(u32 *mantissa, s32 bits) {
    s32 word = bits / 32;
    const s32 place = 31 - bits % 32;
    s32 carry = 0;
    if (mantissa[word] & shl(1, place) && !CRT_ZeroTail(mantissa, bits + 1)) carry = CRT_IncMan(mantissa, bits - 1);
    mantissa[word] &= shl(0xffffffffu, place);
    for (++word; word < 3; ++word) mantissa[word] = 0;
    return carry;
}

void __cdecl CRT_CopyMan(u32 *to, const u32 *from) {
    for (s32 i = 0; i < 3; ++i) to[i] = from[i];
}

s32 __cdecl CRT_IsZeroMan(const u32 *mantissa) {
    for (s32 i = 0; i < 3; ++i)
        if (mantissa[i]) return 0;
    return 1;
}

void __cdecl CRT_ShrMan(u32 *mantissa, s32 bits) {
    const s32 words = bits / 32;
    const s32 count = bits % 32;
    const u32 low = ~shl(0xffffffffu, count);
    u32 carried = 0;
    for (s32 i = 0; i < 3; ++i) {
        const u32 word = mantissa[i];
        mantissa[i] = shr(word, count) | carried;
        carried = shl(word & low, 32 - count);
    }
    for (s32 i = 2; i >= 0; --i) mantissa[i] = i >= words ? mantissa[i - words] : 0;
}

s32 __cdecl CRT_ld12cvt(const u8 *from, u32 *to, const s32 *format) {
    u16 first, top;
    u32 mantissa[3], kept[3];
    std::memcpy(&first, from, 2);
    std::memcpy(&mantissa[1], from + 2, 4);
    std::memcpy(&mantissa[0], from + 6, 4);
    std::memcpy(&top, from + 10, 2);
    mantissa[2] = static_cast<u32>(first) << 16;
    s32 exponent = (top & 0x7fff) - 0x3fff;
    s32 result = 0;
    s32 stored = 0;
    if (exponent == -0x3fff) {
        if (!CRT_IsZeroMan(mantissa)) {
            zero_mantissa(mantissa);
            result = 2;
        }
    } else {
        CRT_CopyMan(kept, mantissa);
        if (CRT_RoundMan(mantissa, format[2])) ++exponent;
        if (exponent < format[1] - format[2]) {
            zero_mantissa(mantissa);
            result = 2;
        } else if (exponent <= format[1]) {
            CRT_CopyMan(mantissa, kept);
            CRT_ShrMan(mantissa, format[1] - exponent);
            CRT_RoundMan(mantissa, format[2]);
            CRT_ShrMan(mantissa, format[3] + 1);
            result = 2;
        } else if (exponent >= format[0]) {
            zero_mantissa(mantissa);
            mantissa[0] |= 0x80000000;
            CRT_ShrMan(mantissa, format[3]);
            stored = format[5] + format[0];
            result = 1;
        } else {
            stored = format[5] + exponent;
            mantissa[0] &= 0x7fffffff;
            CRT_ShrMan(mantissa, format[3]);
        }
    }
    const u32 high = shl(static_cast<u32>(stored), 31 - format[3]) | (top & 0x8000 ? 0x80000000u : 0) | mantissa[0];
    if (format[4] == 0x40) {
        to[1] = high;
        to[0] = mantissa[1];
    } else if (format[4] == 0x20) {
        to[0] = high;
    }
    return result;
}

s32 __cdecl CRT_ld12tod(const u8 *from, u32 *to) { return CRT_ld12cvt(from, to, reinterpret_cast<const s32 *>((uintptr_t)0x00781728)); }

s32 __cdecl CRT_ld12tof(const u8 *from, u32 *to) { return CRT_ld12cvt(from, to, reinterpret_cast<const s32 *>((uintptr_t)0x00781740)); }

}  // extern "C"
