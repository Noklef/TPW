#include "moviepixels.h"

#include <cstdint>

namespace {

u32 table(u32 base, u32 index) { return *reinterpret_cast<const u32 *>((uintptr_t)(base + index * 4)); }

u32 doubled(u32 low, u32 high) { return high << 24 | high << 16 | low << 8 | low; }

// A sum's pixel: its three fields through their tables.
u32 pixel(u32 sum) { return table(0x00fba730, sum >> 23) + table(0x00fbabf8, (sum & 0xff800) >> 11) + table(0x00fbb010, sum & 0x1ff); }

// A channel table (MoviePixels_BuildTables): count entries for v = first, first + 2, ...
void channel_table(u32 at, s32 first, u32 count, const u32 *format, u32 channel, u8 *player) {
    for (u32 k = 0; k < count; ++k, at += 4) {
        const s32 v = first + (s32)k * 2;
        const s32 brightness = *reinterpret_cast<const s32 *>(*reinterpret_cast<u8 *const *>(player) + 0x78);
        s32 level = (s32)((u32)brightness * (u32)v) / 100;
        level = level < 0 ? 0 : level > 0xff ? 0xff : level;
        const u32 bits = format[channel], down = (8 - bits) & 31;
        const s32 truncated = level >> down;
        s32 rounded = (s32)((1u << ((7 - bits) & 31)) + (u32)level) >> down;
        const s32 top = (s32)((1u << (bits & 31)) - 1);
        if (rounded > top) rounded = top;
        u32 &entry = *reinterpret_cast<u32 *>((uintptr_t)at);
        entry = (u32)rounded << (format[3 + channel] & 31);
        if (format[7] <= 0x10) entry = ((u32)truncated << ((format[3 + channel] + 0x10) & 31)) + ((u32)rounded << (format[3 + channel] & 31));
    }
}

}  // namespace

extern "C" {

void __cdecl MoviePixels_Double(const u8 *source, u8 *out, u32 count) {
    do {
        const u32 v = *reinterpret_cast<const u32 *>(source);
        source += 4;
        reinterpret_cast<u32 *>(out)[0] = doubled(v & 0xff, v >> 8 & 0xff);
        reinterpret_cast<u32 *>(out)[1] = doubled(v >> 16 & 0xff, v >> 24);
        out += 8;
        count -= 4;
    } while (count);
}

void __fastcall MoviePixels_BuildTables(const u32 *format, u8 *player) {
    for (s32 i = -0x100; i < 0x100; ++i) {
        const s32 c = i < -0x40 ? -0x40 : i > 0x3f ? 0x3f : i;
        const u32 at = ((u32)(i - 0xc0) & 0x1ff) * 4;
        *reinterpret_cast<u32 *>((uintptr_t)(0x00fb8f30 + at)) = (((u32)c << 12) + (u32)c << 11) + (u32)c + 0x4883d8a9;
        const double x = (double)(float)c - -0.5;
        const u32 a = (u32)(s32)(0.5 - x * 0.3441), b = (u32)(s32)(0.5 - x * -1.772);
        *reinterpret_cast<u32 *>((uintptr_t)(0x00fb9730 + at)) = ((a & 0x1ff) << 11) + (b & 0x1ff);
        const u32 d = (u32)(s32)(0.5 - x * 0.7141), e = (u32)(s32)(0.5 - x * -1.402);
        *reinterpret_cast<u32 *>((uintptr_t)(0x00fb9f30 + at)) = ((d & 0x1ff) + ((e & 0x1ff) << 12)) << 11;
    }
    channel_table(0x00fba730, -0xb1, 0x132, format, 0, player);
    channel_table(0x00fbabf8, -0x85, 0x106, format, 1, player);
    channel_table(0x00fbb010, -0xe1, 0x162, format, 2, player);
}

void __cdecl MoviePixels_FromYuv(const u32 *source, u32 *out, u32 count) {
    do {
        const u32 v = *source++;
        const u32 chroma = table(0x00fb9f30, v & 0xff) + table(0x00fb9730, v >> 8 & 0xff);
        const u32 first = pixel(chroma + table(0x00fb8f30, v >> 24));
        const u32 second = pixel(chroma + table(0x00fb8f30, v >> 16 & 0xff));
        *out++ = (second & 0xffff0000) | (first & 0xffff);
    } while (--count);
}
}
