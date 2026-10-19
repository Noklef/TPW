// Colours to pixels: see color.h.
#include "color.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const ColorEnv kGameEnv = {{
    at<PixelFormat *>(0x00fa7d40),
    at<PixelFormat *>(0x00fa7de8),
    at<PixelFormat *>(0x00fa7e18),
    at<PixelFormat *>(0x00fa7d88),
    at<PixelFormat *>(0x00fa7d70),
    at<PixelFormat *>(0x00fa7e00),
    at<PixelFormat *>(0x00fa7dd0),
    at<PixelFormat *>(0x00fa7db8),
    at<PixelFormat *>(0x00fa7d58),
    at<PixelFormat *>(0x00fa7da0),
    at<PixelFormat *>(0x00fa7d28),
}};
const ColorEnv *g_env = &kGameEnv;

void set(int which, u32 red, u32 green, u32 blue, u32 alpha, u32 bitCount) {
    PixelFormat *const f = g_env->format[which];
    f->red = red;
    f->green = green;
    f->blue = blue;
    f->alpha = alpha;
    f->bitCount = bitCount;
    f->paletted = 0;
}

bool is(const PixelFormat *f, int which) { return PixelFormat_Equal(f, nullptr, g_env->format[which]) != 0; }

u32 rotl(u32 v, u32 n) {
    n &= 31;
    return n ? v << n | v >> (32 - n) : v;
}
u32 rotr(u32 v, u32 n) {
    n &= 31;
    return n ? v >> n | v << (32 - n) : v;
}

// How far to rotate mask so its top bit lands on bit top (mod 32, but only when top is below it).
u32 rotation(u32 top, u32 mask) {
    if (!mask) return 0;
    u32 bit = 31;
    for (u32 probe = 0x80000000u; !(probe & mask); probe >>= 1) --bit;
    return top >= bit ? top - bit : top - bit + 32;
}

}  // namespace

void Color_SetEnv(const ColorEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void __cdecl PixelFormat_InitArgb8888Unused() { set(kFormatArgb8888Unused, 0xff0000, 0xff00, 0xff, 0xff000000, 32); }
void __cdecl PixelFormat_InitArgb1555() { set(kFormatArgb1555, 0x7c00, 0x3e0, 0x1f, 0x8000, 16); }
void __cdecl PixelFormat_InitRgb555() { set(kFormatRgb555, 0x7c00, 0x3e0, 0x1f, 0, 16); }
void __cdecl PixelFormat_InitRgb565() { set(kFormatRgb565, 0xf800, 0x7e0, 0x1f, 0, 16); }
void __cdecl PixelFormat_InitArgb4444() { set(kFormatArgb4444, 0xf00, 0xf0, 0xf, 0xf000, 16); }
void __cdecl PixelFormat_InitRgb888() { set(kFormatRgb888, 0xff0000, 0xff00, 0xff, 0, 24); }
void __cdecl PixelFormat_InitBgr888() { set(kFormatBgr888, 0xff, 0xff00, 0xff0000, 0, 24); }
void __cdecl PixelFormat_InitArgb8888() { set(kFormatArgb8888, 0xff0000, 0xff00, 0xff, 0xff000000, 32); }
void __cdecl PixelFormat_InitAbgr8888() { set(kFormatAbgr8888, 0xff, 0xff00, 0xff0000, 0xff000000, 32); }
void __cdecl PixelFormat_InitXrgb8888() { set(kFormatXrgb8888, 0xff0000, 0xff00, 0xff, 0, 32); }
void __cdecl PixelFormat_InitXbgr8888() { set(kFormatXbgr8888, 0xff, 0xff00, 0xff0000, 0, 32); }

u32 __fastcall Color_ToPixel(Color *c, void *, const PixelFormat *f) {
    const u32 v = c->argb;
    if (is(f, kFormatArgb1555))
        return (((((v >> 7 & 0x1000000) | (v & 0xf80000)) >> 3 | (v & 0xf800)) >> 3) | (v & 0xf8)) >> 3;
    if (is(f, kFormatRgb555)) return (((((v >> 3) & 0x1f0000) | (v & 0xf800)) >> 3) | (v & 0xf8)) >> 3;
    if (is(f, kFormatRgb565)) return (((((v >> 3) & 0x1f0000) | (v & 0xfc00)) >> 2) | (v & 0xf8)) >> 3;
    if (is(f, kFormatArgb4444))
        return (((((v >> 4 & 0xf000000) | (v & 0xf00000)) >> 4 | (v & 0xf000)) >> 4) | (v & 0xf0)) >> 4;
    if (is(f, kFormatArgb8888)) return v;
    if (is(f, kFormatAbgr8888)) return v | (v & 0xff) << 16 | (v >> 16 & 0xff);
    if (is(f, kFormatXrgb8888) || is(f, kFormatRgb888)) return v & 0xffffff;
    if (is(f, kFormatXbgr8888) || is(f, kFormatBgr888)) return Color_SwapRedBlue(c) & 0xffffff;
    ColorShifts_Compute(&c->shifts, nullptr, 0x17, 0xf, 7, 0x1f, f);
    return Color_Shift(c, nullptr, &c->shifts);
}

u32 __fastcall PixelFormat_Equal(const PixelFormat *f, void *, const PixelFormat *other) {
    return f->red == other->red && f->green == other->green && f->blue == other->blue &&
           f->alpha == other->alpha && f->bitCount == other->bitCount && !f->paletted == !other->paletted;
}

void __fastcall ColorShifts_Compute(ColorShifts *s, void *, u32 red, u32 green, u32 blue, u32 alpha,
                                    const PixelFormat *f) {
    s->rotate[0] = rotation(red, f->red);
    s->rotate[1] = rotation(green, f->green);
    s->rotate[2] = rotation(blue, f->blue);
    s->rotate[3] = rotation(alpha, f->alpha);
    s->mask[0] = rotl(f->red, s->rotate[0]);
    s->mask[1] = rotl(f->green, s->rotate[1]);
    s->mask[2] = rotl(f->blue, s->rotate[2]);
    s->mask[3] = rotl(f->alpha, s->rotate[3]);
}

u32 __fastcall Color_SwapRedBlue(const Color *c) {
    const u32 v = c->argb;
    return v | (v & 0xff) << 16 | (v >> 16 & 0xff);
}

u32 __fastcall Color_Shift(const Color *c, void *, const ColorShifts *s) {
    return rotr(c->argb & s->mask[0], s->rotate[0]) | rotr(c->argb & s->mask[1], s->rotate[1]) |
           rotr(c->argb & s->mask[2], s->rotate[2]) | rotr(c->argb & s->mask[3], s->rotate[3]);
}

u32 *__cdecl Colour_Blend3(u32 *out, s32 position, const u32 *a, const u32 *b, const u32 *c) {
    if (position < 0) position = 0;
    else if (position > 100) position = 100;
    const u32 *from, *to;
    s32 fraction;
    if (position < 0x32) {
        from = a;
        to = b;
        fraction = (position << 16) / 0x32;
    } else {
        from = b;
        to = c;
        fraction = ((position - 0x32) << 16) / 0x32;
    }
    u8 record[36];
    for (u32 i = 0; i < 9; ++i)
        reinterpret_cast<u32 *>(record)[i] = from[i];
    const u8 *start = reinterpret_cast<const u8 *>(from);
    const u8 *end = reinterpret_cast<const u8 *>(to);
    for (u32 k = 0; k < 3; ++k)
        record[k] = (u8)(record[k] + (u8)((((s32)end[k] - (s32)start[k]) * fraction) >> 16));
    record[3] = 0xff;
    for (u32 i = 0; i < 9; ++i)
        out[i] = reinterpret_cast<u32 *>(record)[i];
    return out;
}

void __cdecl Format_BuildChannelTable(u32 *table, u32, u32, u32, u32, u32 red, u32 green, u32 blue, u32 alpha) {
    const u32 masks[4] = {alpha, red, green, blue};
    u32 width[4], shift[4];
    for (int k = 0; k < 4; k++) {
        width[k] = 0;
        for (u32 m = masks[k]; m; m >>= 1)
            width[k] += m & 1;
        shift[k] = 0;
        if (masks[k]) {
            for (u32 m = masks[k]; !(m & 1); m >>= 1)
                shift[k]++;
        }
    }
    for (s32 i = 0; i < 0x100; i++) {
        for (int k = 0; k < 4; k++)
            table[i * 4 + k] = static_cast<u32>(i >> ((8 - width[k]) & 0x1f)) << (shift[k] & 0x1f);
    }
}
}  // extern "C"

namespace {
struct PixelSums {
    s32 partial, alpha, red, green, blue;
};

// One 4-byte pixel (bytes blue, green, red, alpha) through the table, stored as a dword; the sums read the bytes again after the store.
void convert4(const u8 *p, u8 *dst, const u32 *table, PixelSums &s) {
    const u8 a = p[3];
    if (a < 0xf0 && a != 0)
        s.partial++;
    const u32 value = table[p[2] * 4 + 1] | table[p[1] * 4 + 2] | table[p[0] * 4 + 3] | table[a * 4];
    *reinterpret_cast<u32 *>(dst) = value;
    s.alpha += p[3];
    s.red += p[2];
    s.green += p[1];
    s.blue += p[0];
}

// One 3-byte pixel (blue, green, red) with the alpha record of 0xff.
void convert3(const u8 *p, u8 *dst, const u32 *table, PixelSums &s) {
    const u32 value = table[p[2] * 4 + 1] | table[p[0] * 4 + 3] | table[p[1] * 4 + 2] | table[0xff * 4];
    *reinterpret_cast<u32 *>(dst) = value;
    s.red += p[2];
    s.green += p[1];
    s.blue += p[0];
}

const u8 *offset(const u8 *base, u32 bytes) { return reinterpret_cast<const u8 *>(static_cast<std::uintptr_t>(static_cast<u32>(reinterpret_cast<std::uintptr_t>(base)) + bytes)); }
}  // namespace

extern "C" {
u32 __cdecl Format_ConvertPixels(u8 *dst, const u8 *src, s32 width, s32 height, const u32 *table, s32 step, s32 bytes, u32 flags) {
    const s32 count = (s32)((u32)height * (u32)width);
    PixelSums s = {0, 0, 0, 0, 0};
    u32 result = 0;
    if (bytes == 4) {
        result = 1;
        if (flags & 1) {
            if (height > 0) {
                u32 row = (u32)count * 4;
                for (s32 rows = height; rows != 0; rows--) {
                    if (width > 0) {
                        const u8 *p = offset(src, row - (u32)width * 4);
                        for (s32 n = width; n != 0; n--, p += 4, dst += step)
                            convert4(p, dst, table, s);
                    }
                    row -= (u32)width * 4;
                }
            }
        } else {
            for (s32 i = 0; i < count; i++, dst += step)
                convert4(offset(src, (u32)i * 4), dst, table, s);
        }
    } else if (flags & 1) {
        if (height > 0) {
            u32 row = (u32)count * 3;
            for (s32 rows = height; rows != 0; rows--) {
                if (width > 0) {
                    const u8 *p = offset(src, row - (u32)width * 3);
                    s.alpha = (s32)((u32)s.alpha + (u32)width * 255);
                    for (s32 n = width; n != 0; n--, p += 3, dst += step)
                        convert3(p, dst, table, s);
                }
                row -= (u32)width * 3;
            }
        }
    } else if (count > 0) {
        s.alpha = (s32)((u32)count * 255);
        const u8 *p = src;
        for (s32 n = count; n != 0; n--, p += 3, dst += step)
            convert3(p, dst, table, s);
    }
    const X87Reg limit = X87Reg((double)count) * 0.05f;  // 0x701720
    u32 average = (u32)(s.alpha / width) * (u32)height;
    average = average << 8 | (u32)(s.red / width) * (u32)height;
    average = average << 8 | (u32)(s.green / width) * (u32)height;
    average = average << 8 | (u32)(s.blue / width) * (u32)height;
    at<u32 *>(0x0087a25c)[0] = average;
    if (s.partial >= (s32)(long long)limit.v)  // __ftol, the low half
        result |= 2;
    return result;
}
}  // extern "C"
