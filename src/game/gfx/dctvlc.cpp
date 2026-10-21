#include "dctvlc.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void fill(u32 at, u32 count, u32 value) {
    for (u32 k = 0; k < count; ++k) global<u32>(at + k * 4) = value;
}

// A code's entry: the run / level word as (word << 16 | word & 0xfc00) << 6, the skip in the low bits.
u32 code_entry(u32 word, u32 skip) { return (word << 16 | (word & 0xfc00)) << 6 | skip; }

// Fills count dwords from base + index * 4 when count (a power of two from a shift: may be the sign bit) is above 0.
void spread(u32 base, s32 index, s32 count, u32 value) {
    if (count > 0) fill(base + (u32)index * 4, (u32)count, value);
}

// The bit reader's state (0xfbd64c-0xfbd654), kept in locals over a block.
struct Bits {
    const u16 *next;
    u32 bits;
    s32 count;

    void refill() {
        if (count < 0x10) {
            bits |= (u32)*next++ << ((0x10 - count) & 31);
            count += 0x10;
        }
    }
    void skip(u32 n) {
        bits <<= n & 31;
        count -= (s32)n;
        refill();
    }
};

// value * factor, 16.16, rounded by the product's bit 15 (IMUL; SHL EDX,16; SHR EAX,16; ADC).
u32 fixmul(s32 value, s32 factor) {
    const int64_t product = (int64_t)value * factor;
    const u32 low = (u32)product, high = (u32)((uint64_t)product >> 32);
    return (high << 16) + (low >> 16) + (low >> 15 & 1);
}

}  // namespace

extern "C" {

void __cdecl DctVlc_BuildTables() {
    global<u32>(0x00fbc44c) = 0xf;
    fill(0x00fbc450, 7, 0x1f);
    fill(0x00fbc46c, 8, 0x2f);
    fill(0x00fbc84c, 0x80, 0x3f);
    for (u32 record = 0x007061d8; record < 0x007067b8; record += 0x10) {
        const u32 length = global<u32>(record), word = global<u32>(record + 4);
        const s32 code = global<s32>(record + 0xc);
        if (code & 0xfc00)
            spread(0x00fbc44c, code >> 7, (s32)(1u << ((9 - length) & 31)), code_entry(word, length));
        else
            spread(0x00fbd04c, code >> 2, (s32)(1u << ((0xe - length) & 31)), code_entry(word, length - 6));
    }
    for (u32 record = 0x007067b8; record < 0x00706fb8; record += 0x10) {
        const u32 length = global<u32>(record) + 8, word = global<u32>(record + 4);
        const s32 code = global<s32>(record + 0xc);
        if (!(code & 0x8000))
            spread(0x00fbcc4c, code >> 7, (s32)(1u << ((0x11 - length) & 31)), code_entry(word, length - 9));
        else
            spread(0x00fbd04c, code >> 10, (s32)(1u << ((0xe - length) & 31)), code_entry(word, length - 6));
    }
    fill(0x00fbd44c, 0x20, 1);
    for (u32 k = 0; k < 0x10; ++k) global<u32>(0x00fbd4cc + k * 4) = (k + 1) << 22 | 6;
    for (u32 k = 0; k < 0x10; ++k) global<u32>(0x00fbd50c + k * 4) = (0xfc000000u + (k << 22)) | 6;
    global<u32>(0x00faac68) = 1;
}

s32 __cdecl DctVlc_DecodeBlock() {
    Bits in{global<const u16 *>(0x00fbd64c), global<u32>(0x00fbd650), global<s32>(0x00fbd654)};
    global<s32>(0x00fbc14c) = ((s32)in.bits >> 24) * global<s32>(0x00fbd54c);
    in.skip(8);
    fill(0x00fbc150, 0x3f, 0);
    s32 k = 1;
    for (;;) {
        u32 entry = global<u32>(0x00fbc44c + (in.bits >> 23) * 4);
        const u8 kind = (u8)entry;
        if ((s8)kind > 9) {
            if (kind >= 0x30) {   // the end
                in.skip(2);
                break;
            }
            if (kind >= 0x20) {   // the escape: the run and level raw
                in.skip(6);
                entry = (in.bits & 0xffff0000) | 0x10;
            } else if (kind >= 0x10) {
                in.skip(6);
                entry = global<u32>(0x00fbd04c + (in.bits >> 24) * 4);
            } else {
                in.skip(9);
                entry = global<u32>(0x00fbcc4c + (in.bits >> 24) * 4);
            }
        }
        k += (s32)(entry >> 16 & 0x3f);
        in.skip((u8)entry);
        const u32 offset = global<u32>(0x00fbc34c + (u32)k * 4);
        ++k;
        global<s32>(0x00fbc14c + offset) = ((s32)entry >> 22) * global<s32>(0x00fbd54c + offset);
    }
    global<const u16 *>(0x00fbd64c) = in.next;
    global<u32>(0x00fbd650) = in.bits;
    global<s32>(0x00fbd654) = in.count;
    return k;
}

void __fastcall DctVlc_Skip(u32 n) {
    const u32 count = global<u32>(0x00fbd654) - n;
    global<u32>(0x00fbd654) = count;
    global<u32>(0x00fbd650) <<= n & 31;
    if (count >= 0x10) return;
    const u16 *next = global<const u16 *>(0x00fbd64c);
    global<u32>(0x00fbd654) = count + 0x10;
    global<u32>(0x00fbd650) |= (u32)*next << ((0x10 - count) & 31);
    global<const u16 *>(0x00fbd64c) = next + 1;
}

s32 __cdecl DctVlc_ReadCode() {
    const u32 entry = global<u32>(0x00fbd44c + (global<u32>(0x00fbd650) >> 26) * 4);
    DctVlc_Skip(entry & 0xff);
    return (s32)entry >> 22;
}

void __fastcall DctVlc_FillDc(s32 *out, s32 pitch) {
    for (s32 row = 0; row < 8; ++row, out += pitch)
        for (s32 k = 0; k < 8; ++k) out[k] = global<s32>(0x00fbc14c);
}

void __fastcall DctVlc_BeginFrame(const u16 *data, s32 *out, s32 quality) {
    if (global<u32>(0x00faac68) == 0) DctVlc_BuildTables();
    const u32 dc = global<u32>(0x00706fb8);
    const u32 bits = (u32)data[0] << 16 | data[1];
    global<u32>(0x00fbd654) = 0x20;
    global<u32>(0x00fbd650) = bits;
    global<const u16 *>(0x00fbd64c) = data + 2;
    global<s32 *>(0x00faac60) = out;
    global<u32>(0x00fbd54c) = fixmul(global<s32>(0x00fbc24c), (s32)(dc << 15));
    for (u32 k = 0; k < 0x3f; ++k)
        global<u32>(0x00fbd550 + k * 4) = fixmul(global<s32>(0x00fbc250 + k * 4), (s32)(global<u32>(0x00706fbc + k * 4) * (u32)quality << 12));
}
}
