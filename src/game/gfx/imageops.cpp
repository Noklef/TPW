#include "imageops.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "array3d.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The window of the radius around i inside [0, size): the maximum (start 0) or minimum (start 0xff) of the bytes in it; for the minimum, 0 within the radius of the ends.
template <bool Max> u8 Window(const u8 *line, s32 i, s32 radius, s32 size) {
    if (!Max && (i < radius || size - radius - 1 < i)) return 0;
    s32 first = i - radius < 0 ? 0 : i - radius;
    s32 last = i + radius;
    if (size - 1 < last) last = size - 1;
    u8 best = Max ? 0 : 0xff;
    for (s32 k = first; k <= last; ++k) {
        if (Max ? best < line[k] : line[k] < best) best = line[k];
    }
    return best;
}

template <bool Max> u32 Filter(u32 *image, s32 radius) {
    const s32 width = (s32)image[0], height = (s32)image[1];
    u8 *const scratch = static_cast<u8 *>(GameHeap_Alloc((u32)(width > height ? width : height)));
    if (!scratch) return 0;
    const s32 pixel = (s32)image[2];
    u8 *const data = reinterpret_cast<u8 *>(image[3]);
    for (s32 row = 0; row < height; ++row) {
        u8 *out = data + width * row * pixel;
        for (s32 i = 0; i < width; ++i) scratch[i] = out[i];
        for (s32 x = 0; x < width; ++x)
            *out++ = Window<Max>(scratch, x, radius, width);
    }
    for (s32 column = 0; column < width; ++column) {
        u8 *const base = data + column * pixel;
        for (s32 y = 0; y < height; ++y) scratch[y] = base[width * y * pixel];
        u8 *out = base;
        for (s32 y = 0; y < height; ++y) {
            *out = Window<Max>(scratch, y, radius, height);
            out += width * pixel;
        }
    }
    GameHeap_Free(scratch);
    return 1;
}

// byte += ((target - byte) * weight) >> 8, as the compiled code: 32-bit arithmetic cut to the low byte.
u8 Blend(u8 byte, u32 target, u32 weight) { return (u8)(((target - byte) * weight) >> 8) + byte; }

}  // namespace

extern "C" {

void __fastcall Pixels_BlendOnto(const u8 *source, void *, u8 *dest, u32 x, u32 y) {
    u8 *out = at<u8 *>(dest, 0xc) + (at<u32>(dest, 0) * y + x) * at<u32>(dest, 8);
    const u8 *in = at<const u8 *>(source, 0xc);
    for (u32 row = 0; row < at<u32>(source, 4); ++row) {
        for (u32 column = 0; column < at<u32>(source, 0); ++column) {
            const s32 alpha = in[0];
            out[0] = 0xff;
            for (int channel = 1; channel < 4; ++channel) {
                const s32 d = out[channel];
                const s32 s = in[channel];
                out[channel] = (u8)(d + (((s - d) * alpha) >> 8));
            }
            in += 4;
            out += 4;
        }
        out += (at<u32>(dest, 0) - at<u32>(source, 0)) * at<u32>(dest, 8);
    }
}

u8 __fastcall Image_ShrinkInto(const u8 *image, void *, u8 *dest, s32 width, s32 height, u32 format) {
    const u32 xstep = ((at<u32>(image, 0) << 16) + 0x8000) / (u32)width;
    const u32 ystep = ((at<u32>(image, 4) << 16) + 0x8000) / (u32)height;
    const u8 *source = at<const u8 *>(image, 0xc);
    u8 *const rows = static_cast<u8 *>(GameHeap_Alloc(((ystep >> 16) + 1) * (u32)width * 4));
    if (rows == nullptr) return 0;
    u8 *const xcounts = static_cast<u8 *>(GameHeap_Alloc((u32)width));
    u8 *const ycounts = static_cast<u8 *>(GameHeap_Alloc((u32)height));
    if (xcounts == nullptr || ycounts == nullptr) return 0;  // sic: the other blocks are kept
    u32 position = 0;
    for (s32 i = 0; i < width; i++) {
        const u32 before = position;
        position += xstep;
        xcounts[i] = (u8)((position >> 16) - (before >> 16));
    }
    position = 0;
    for (s32 j = 0; j < height; j++) {
        const u32 before = position;
        position += ystep;
        ycounts[j] = (u8)((position >> 16) - (before >> 16));
    }
    for (s32 j = 0; j < height; j++) {
        const u32 yc = ycounts[j];
        if (yc != 0) {
            u8 *out = rows;
            for (u32 band = yc; band != 0; band--) {
                for (s32 i = 0; i < width; i++) {
                    const u32 xc = xcounts[i];
                    u32 sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
                    for (u32 k = xc; k != 0; k--) {
                        sum0 += source[0];
                        sum1 += source[1];
                        sum2 += source[2];
                        sum3 += source[3];
                        source += 4;
                    }
                    out[0] = (u8)((sum0 & 0xffff) / xc);  // (a count of 0 divides by 0, sic)
                    out[1] = (u8)((sum1 & 0xffff) / xc);
                    out[2] = (u8)((sum2 & 0xffff) / xc);
                    out[3] = (u8)((sum3 & 0xffff) / xc);
                    out += 4;
                }
            }
        }
        for (s32 i = 0; i < width; i++) {
            const u8 *column = rows + i * 4;
            u32 sum1 = 0, sum2 = 0, sum3 = 0;  // (the first byte's sum is not used)
            for (u32 k = yc; k != 0; k--) {
                sum1 += column[1];
                sum2 += column[2];
                sum3 += column[3];
                column += (u32)width * 4;
            }
            const u32 c1 = (sum1 & 0xffff) / yc, c2 = (sum2 & 0xffff) / yc, c3 = (sum3 & 0xffff) / yc;
            if (format == 1) {
                dest[0] = (u8)c3;
                dest[1] = (u8)c2;
                dest[2] = (u8)c1;
                dest[3] = 0xff;
                dest += 4;
            } else if (format == 2) {
                const u32 packed = ((c1 & 0xfff0) | 0xff00) << 4 | (c2 & 0xf0) | ((u8)c3 >> 4);
                dest[0] = (u8)packed;
                dest[1] = (u8)(packed >> 8);
                dest += 2;
            }
        }
    }
    GameHeap_Free(ycounts);
    GameHeap_Free(xcounts);
    GameHeap_Free(rows);
    return 1;
}

u8 __fastcall Panel_ShrinkInto(u8 *panel, void *, u8 *dest, s32 width, s32 height, u32 format) {
    if (at<u8 *>(panel, 0x5c) == nullptr) Panel_Reset(panel);  // (sic: fills through the null pixels; Panel_Reset inline)
    return Image_ShrinkInto(panel + 0x50, nullptr, dest, width, height, format);
}

void __fastcall Panel_Reset(u8 *panel) {
    u8 *const target = panel + 0x50;
    u8 *pixel = at<u8 *>(target, 0xc);
    const u32 count = at<u32>(target, 0) * at<u32>(target, 4);
    for (u32 i = 0; i < count; ++i) {
        pixel[0] = 0xff;
        pixel[1] = 0xfb;
        pixel[2] = 0xf8;
        pixel[3] = 0xc6;
        pixel += 4;
    }
    Pixels_BlendOnto(panel + 0x60, nullptr, target, 0, 0);
    Pixels_BlendOnto(panel + 0x20, nullptr, target, 0x220, 0);
    Pixels_BlendOnto(panel + 0x30, nullptr, target, 0x1dd, 6);
    Pixels_BlendOnto(panel + 0x40, nullptr, target, 0x141, 0xda);
    Pixels_BlendOnto(panel + 0x10, nullptr, target, 4, 0x9f);
    Pixels_BlendOnto(panel, nullptr, target, 0xb6, 0);
}

u8 Image_CropInto(u8 *destination, u8 *source, s32 x, s32 y) {
    const s32 height = at<s32>(source, 4), width = at<s32>(source, 0);
    const s32 lastRow = height - 1, lastColumn = width - 1;
    auto start = [](s32 offset, s32 last) { return -offset <= last ? (-offset < 0 ? 0 : -offset) : last; };
    auto end = [](s32 size, s32 offset, s32 last) { const s32 e = size - offset - 1; return e > last ? last : (e < 0 ? 0 : e); };
    const s32 firstRow = start(y, lastRow), endRow = end(height, y, lastRow);
    const s32 firstColumn = start(x, lastColumn), endColumn = end(width, x, lastColumn);
    Array3D_Realloc(destination, nullptr, (u32)width, (u32)height, 1);
    const s32 stride = at<s32>(destination, 0) * at<s32>(destination, 8);
    u8 *row = at<u8 *>(destination, 0xc) + stride * (y < 0 ? 0 : y) + (x < 0 ? 0 : x);
    for (s32 line = firstRow; line <= endRow; ++line) {
        const u8 *from = at<u8 *>(source, 0xc) + (at<s32>(source, 0) * line + firstColumn) * at<s32>(source, 8);
        u8 *to = row;
        if (firstColumn <= endColumn) {
            for (s32 n = endColumn - firstColumn + 1; n != 0; --n) {
                *to++ = *from;
                from += at<s32>(source, 8);
            }
        }
        row += stride;
    }
    return 1;
}

u32 __cdecl Image_Dilate(u32 *image, s32 radius) { return Filter<true>(image, radius); }

u32 __cdecl Image_Erode(u32 *image, s32 radius) { return Filter<false>(image, radius); }

void __cdecl Image_MinMark(u32 *image, const u32 *, u8 mark, const u32 *maskImage) {
    const s32 width = (s32)image[0];
    u8 *current = reinterpret_cast<u8 *>(image[3]);
    u8 *mask = reinterpret_cast<u8 *>(maskImage[3]);
    for (s32 y = 0; y < (s32)image[1]; ++y) {
        for (s32 x = 0; x < width && x < (s32)image[0]; ++x) {  // (the width is read again for each pixel)
            const u8 original = *current;
            u8 least = original;
            const u8 *above = current - 2 * width;
            const u8 *below = current + 2 * width;
            auto take = [&least](u8 value) { if (value < least) least = value; };
            const bool left = x > 0, right = x < (s32)image[0] - 2;
            if (y > 0) {
                if (left) take(above[-2]);
                take(above[0]);
                if (right) take(above[2]);
            }
            if (left) take(current[-2]);
            if (right) take(current[2]);
            if (y < (s32)image[1] - 2) {
                if (left) take(below[-2]);
                take(below[0]);
                if (right) take(below[2]);
            }
            current[1] = least;
            if (least != original) *mask = mark;
            current += 2;
            ++mask;
        }
    }
    u8 *to = reinterpret_cast<u8 *>(image[3]);
    for (s32 i = 0; i < (s32)image[0] * (s32)image[1]; ++i) {
        to[0] = to[1];
        to += 2;
    }
}

void __fastcall Pixels_Stamp(u8 *source, void *, u8 *destination, s32 x, s32 y) {
    const u8 *from = at<u8 *>(source, 0xc);
    u8 *to = at<u8 *>(destination, 0xc) + ((at<s32>(destination, 0) * y + x) * at<s32>(destination, 8));
    for (s32 row = 0; row < at<s32>(source, 4); ++row) {
        for (s32 column = 0; column < at<s32>(source, 0); ++column) {
            const s32 pixel = at<s32>(source, 8);
            if (pixel == 1) {
                const u32 v = from[0];
                to[0] = Blend(to[0], v, v);
                to[1] = Blend(to[1], from[0], v);
                to[2] = Blend(to[2], from[0], v);
                to[3] = Blend(to[3], from[0], v);
                ++from;
            } else if (pixel == 2) {
                to[0] = 0xff;
                to[1] = from[0];
                to[2] = from[1];
                to[3] = 0;
                from += 2;
            } else {
                const u32 alpha = from[0];
                to[0] = from[0] > to[0] ? from[0] : to[0];
                to[1] = Blend(to[1], from[1], alpha);
                to[2] = Blend(to[2], from[2], alpha);
                to[3] = Blend(to[3], from[3], alpha);
                from += 4;
            }
            to += 4;
        }
        to += (at<s32>(destination, 0) - at<s32>(source, 0)) * at<s32>(destination, 8);
    }
}

void __fastcall Mask_Tint(u8 *mask, void *, u8 *destination, u32 scale, u32 c1, u32 c2, u32 c3) {
    const u8 *from = at<u8 *>(mask, 0xc);
    if (!from) return;
    u8 *to = at<u8 *>(destination, 0xc);
    const u32 s = scale & 0xff;
    const u8 t1 = (u8)(((c1 & 0xff) * s) >> 8), t2 = (u8)(((c2 & 0xff) * s) >> 8), t3 = (u8)(((c3 & 0xff) * s) >> 8);
    for (s32 row = 0; row < at<s32>(mask, 4); ++row) {
        for (s32 column = 0; column < at<s32>(mask, 0); ++column) {
            const u8 a = (u8)((*from++ * s) >> 8);
            to[0] = a > to[0] ? a : to[0];
            to[1] = Blend(to[1], t1, a);
            to[2] = Blend(to[2], t2, a);
            to[3] = Blend(to[3], t3, a);
            to += 4;
        }
        to += (at<s32>(destination, 0) - at<s32>(mask, 0)) * at<s32>(destination, 8);
    }
}
void __fastcall Image_ErosionRamp(void *, void *, const u32 *a, u32 count, const u32 *b) {
    const u32 width = a[0], height = a[1];
    u8 *const work = static_cast<u8 *>(GameHeap_Alloc(height * width * 2));
    const u8 *from = reinterpret_cast<const u8 *>(a[3]);
    u8 *mask = reinterpret_cast<u8 *>(b[3]);
    u8 *to = work;
    for (s32 i = 0; i < (s32)(a[0] * a[1]); i++) {
        to[0] = from[i] ? 0xff : 0;
        to[1] = from[i] ? 0xff : 0;
        mask[i] = 0;
        to += 2;
    }
    u32 image[4] = {width, height, 2, (u32)(uintptr_t)work};
    if (count) {
        const float total = (float)(double)count;
        for (u32 i = 0; i < count; i++) {
            const X87Reg t = X87_SubR(X87Reg((double)i) / total, 1.0f);   // 0x702f40
            const X87Reg t2 = t * t, t3 = t * t2;
            const X87Reg ramp = (t3 + t3 - t2 * 3.0f) * 256.0f;   // 0x702f48, 0x702f4c
            Image_MinMark(image, a, (u8)(s32)(long long)ramp.v, b);   // (__ftol's low byte)
        }
    }
    u8 *out = reinterpret_cast<u8 *>(b[3]);
    for (s32 i = 0; i < (s32)(width * height); i++)
        out[i] |= work[2 * i];
    if (work)
        GameHeap_Free(work);
}
}  // extern "C"
