// Palette helpers: see palette.h.
#include "palette.h"

namespace {

s32 square(s32 v) { return v * v; }
s32 absolute(s32 v) { return v < 0 ? -v : v; }

}  // namespace

extern "C" {

void __cdecl Palette_ExpandRgb(u8 *dst, const u8 *src, u32 count) {
    for (; count; --count, dst += 4, src += 3) {
        dst[0] = src[0];
        dst[1] = src[1];
        const u8 blue = src[2];
        dst[3] = 0;
        dst[2] = blue;
    }
}

u8 __cdecl Palette_Nearest(const u8 *palette, u32 red, u32 green, u32 blue) {
    const s32 r = static_cast<s32>(red & 0xff);
    const s32 g = static_cast<s32>(green & 0xff);
    const s32 b = static_cast<s32>(blue & 0xff);
    const auto distance = [&](u32 i) {
        const u8 *const e = palette + 4 * i;
        return square(g - e[1]) + square(b - e[2]) + square(r - e[0]);
    };
    s32 best = 0x1000000;
    for (u32 i = 0; i < 0x100; ++i) {
        const s32 d = distance(i);
        if (d < best) {
            best = d;
            if (d == 0) return static_cast<u8>(i);
        }
    }
    u8 nearest[0x100];
    u32 count = 0;
    for (u32 i = 0; i < 0x100; ++i) {
        if (distance(i) == best) nearest[count++] = static_cast<u8>(i);
    }
    if (static_cast<u16>(count) == 1) return nearest[0];

    const auto difference = [&](u8 i) {
        const u8 *const e = palette + 4 * i;
        return absolute(g - e[1]) + absolute(b - e[2]) + absolute(r - e[0]);
    };
    best = 0x1000000;
    for (u32 j = 0; j < count; ++j) {
        const s32 d = difference(nearest[j]);
        if (d < best) best = d;
    }
    u8 closest[0x100];
    u32 ties = 0;
    for (u32 j = 0; j < count; ++j) {
        if (difference(nearest[j]) == best) closest[ties++] = nearest[j];
    }
    if (static_cast<u16>(ties) == 1) return closest[0];

    s32 darkest = 0x1000000;
    u8 result = 0;
    for (u32 j = 0; j < ties; ++j) {
        const u8 *const e = palette + 4 * closest[j];
        const s32 weight = square(e[2]) + 2 * (square(e[1]) + square(e[0]));
        if (weight < darkest) {
            darkest = weight;
            result = closest[j];
        }
    }
    return result;
}

void __fastcall PalColour_Set(u8 *colour, void *, u32 red, u32 green, u32 blue) {
    colour[2] = (u8)red;
    colour[1] = (u8)green;
    colour[0] = (u8)blue;
    colour[3] = 0xff;
    colour[4] = Palette_Nearest(reinterpret_cast<const u8 *>(0x00fa71f8), red, green, blue);
}

}  // extern "C"
