#include "tgqvideo.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

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
