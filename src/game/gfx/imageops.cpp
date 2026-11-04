#include "imageops.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "array3d.h"
#include "imagecodec.h"
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

// A colour mask's shifts: down by its lowest set bit, up by 8 - the run of set bits from it (as the shift count, modulo 32). A mask of 0
// never ends, sic.
void MaskShifts(u32 mask, u32 &down, u32 &up) {
    u32 zeros = 0, ones = 0;
    while ((mask & 1) == 0) {
        mask >>= 1;
        ++zeros;
    }
    while ((mask & 1) != 0) {
        mask >>= 1;
        ++ones;
    }
    down = zeros;
    up = (8 - ones) & 31;
}

// Image_ShrinkSurface's band, averaged down into the image's pixels: 0xff, then bytes 1-3. Returns byte 0's sum for the last column.
u32 ShrinkBandDown(const u8 *rows, u32 yc, u8 *&out) {
    u32 sum0 = 0;
    for (u32 i = 0; i < 320; ++i) {
        const u8 *column = rows + i * 4;
        u32 sum1 = 0, sum2 = 0, sum3 = 0;
        sum0 = 0;
        for (u32 k = yc; k != 0; --k) {
            sum0 += column[0];
            sum1 += column[1];
            sum2 += column[2];
            sum3 += column[3];
            column += 0x500;
        }
        out[0] = 0xff;
        out[1] = (u8)((sum1 & 0xffff) / yc);  // (a count of 0 divides by 0, sic)
        out[2] = (u8)((sum2 & 0xffff) / yc);
        out[3] = (u8)((sum3 & 0xffff) / yc);
        out += 4;
    }
    return sum0;
}

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

namespace {
// exp(x) as the original's inline code has it: fldl2e; fmulp; frndint; f2xm1; fld1; faddp; fscale, the two fmulp and faddp at the 24 bits of precision control.
double X87_ExpInline(double x) {
    const double y = X87_MulDouble(x, 0x1.71547652b82fep+0);  // log2(e)
    const double whole = std::rint(y);
    return std::ldexp(X87_Round(std::pow(2.0, y - whole)), (int)whole);
}

s32 GaussianSize(double sigma) { return 3 - (s32)(long long)X87_MulDouble(sigma, -2.5); }

// The kernel: a size x size square (rows 1: size wide, a line), its sum 1.
double *GaussianKernel(s32 size, s32 rows, double sigma) {
    double *const kernel = static_cast<double *>(GameHeap_Alloc(rows * size * 8));
    const s32 half = (size - 1) / 2, rowHalf = (rows - 1) / 2 == 0 && rows == 1 ? 0 : half;
    const X87Reg variance = X87Reg(X87_MulDouble(sigma, sigma));
    const X87Reg divisor = X87Reg(X87_MulDouble(variance.v, *reinterpret_cast<const double *>((uintptr_t)0x00702f28)));
    X87Reg sum = X87Reg(0.0);
    double *cell = kernel;
    for (s32 row = 0; row < rows; row++) {
        for (s32 column = 0; column < size; column++) {
            const s32 distance = (column - half) * (column - half) + (row - rowHalf) * (row - rowHalf);
            const X87Reg power = X87Reg(X87_MulDouble((double)distance, -0.5)) / variance;
            const X87Reg value = X87Reg(X87_ExpInline(power.v)) / divisor;
            sum = sum + value;
            *cell++ = value.v;
        }
    }
    for (s32 i = 0; i < rows * size; i++) kernel[i] = (X87Reg(kernel[i]) / sum).v;
    return kernel;
}

// The pixel at i of a line of count bytes through the line kernel: what falls off the line's ends is left out, not made up.
u8 BlurredInLine(const u8 *line, s32 i, s32 count, const double *kernel, s32 size, s32 half) {
    const s32 first = i - half < 0 ? 0 : i - half;
    const s32 last = i + size - half - 1 > count - 1 ? count - 1 : i + size - half - 1;
    X87Reg sum = X87Reg(0.0);
    const double *cell = kernel + (first + half - i);
    for (s32 at = first; at <= last; at++) sum = sum + X87Reg((double)line[at]) * X87Reg(*cell++);
    return (u8)(s32)(long long)sum.v;  // (__ftol's low byte)
}

// The first entry moved to the end (where the caller overwrites or refills it).
void *RotateDown(void *listIn, s32 count) {
    void **const list = static_cast<void **>(listIn);
    void *const first = list[0];
    for (s32 i = 0; i + 1 < count; i++) list[i] = list[i + 1];
    list[count - 1] = first;
    return first;
}
}  // namespace

void __fastcall Image_GaussianBlur(void *, void *, const u32 *image, const u32 *mask, float sigma) {
    const s32 size = GaussianSize(sigma);
    double *const kernel = GaussianKernel(size, size, sigma);
    const s32 width = (s32)image[0];
    u8 *const data = reinterpret_cast<u8 *>((uintptr_t)image[3]);

    // The rows above and at the one worked on, unblurred (copies, all of the first row to begin with); those below, in the image still.
    const s32 lineCount = (size + 1) / 2;
    u8 **const lines = static_cast<u8 **>(GameHeap_Alloc(lineCount * 4));
    for (s32 i = 0; i < lineCount; i++) {
        lines[i] = static_cast<u8 *>(GameHeap_Alloc(width));
        memcpy(lines[i], data, width);
    }
    const s32 belowCount = size - lineCount;
    const u8 **const below = static_cast<const u8 **>(GameHeap_Alloc(belowCount * 4));
    for (s32 i = 0; i < belowCount; i++) {
        const s32 last = (s32)image[1] - 1;
        below[i] = data + width * (i + 1 < last ? i + 1 : last);
    }

    const s32 half = (size - 1) / 2, end = (size + 2) / 2;
    u8 *to = data;
    const u8 *on = reinterpret_cast<const u8 *>((uintptr_t)mask[3]);
    for (s32 y = 0; y < (s32)image[1]; y++, to += width, on += mask[0]) {
        for (s32 x = 0; x < width; x++) {
            if (!on[x]) continue;
            X87Reg sum = X87Reg(0.0);
            const double *cell = kernel;
            for (s32 dy = -half; dy < end; dy++) {
                const u8 *const row = dy <= 0 ? lines[half + dy] : below[dy - 1];
                for (s32 dx = -half; dx < end; dx++) {
                    const s32 from = x + dx;
                    const s32 inside = from > width - 1 ? width - 1 : (from < 0 ? 0 : from);
                    sum = sum + X87Reg((double)row[inside]) * X87Reg(*cell++);
                }
            }
            to[x] = (u8)(s32)(long long)sum.v;  // (__ftol's low byte)
        }
        memcpy(RotateDown(lines, lineCount), below[0], width);
        RotateDown(below, belowCount);
        const s32 last = (s32)image[1] - 1, next = size / 2 + y + 1;
        below[belowCount - 1] = data + width * (next < last ? next : last);
    }

    for (s32 i = 0; i < lineCount; i++) GameHeap_Free(lines[i]);
    GameHeap_Free(lines);
    GameHeap_Free(below);
    GameHeap_Free(kernel);
}

u8 __cdecl Image_GaussianBlurLines(const u32 *image, double sigma) {
    const s32 size = GaussianSize(sigma), half = (size - 1) / 2;
    double *const kernel = GaussianKernel(size, 1, sigma);
    const s32 width = (s32)image[0], height = (s32)image[1];
    u8 *const line = static_cast<u8 *>(GameHeap_Alloc(width > height ? width : height));
    if (!line) return 0;
    u8 *const data = reinterpret_cast<u8 *>((uintptr_t)image[3]);
    const s32 pixel = (s32)image[2], rowBytes = width * pixel;
    for (s32 y = 0; y < height; y++) {
        u8 *const row = data + y * rowBytes;
        memcpy(line, row, width);
        for (s32 x = 0; x < width; x++) row[x] = BlurredInLine(line, x, width, kernel, size, half);
    }
    for (s32 x = 0; x < width; x++) {
        u8 *const column = data + x * pixel;
        for (s32 y = 0; y < height; y++) line[y] = column[y * rowBytes];
        for (s32 y = 0; y < height; y++) column[y * rowBytes] = BlurredInLine(line, y, height, kernel, size, half);
    }
    GameHeap_Free(line);
    GameHeap_Free(kernel);
    return 1;
}

u8 __fastcall Image_ShrinkSurface(u8 *image, void *, SurfaceDesc desc) {
    const u32 *const d = desc.d;
    const u32 red = d[0x58 / 4], green = d[0x5c / 4], blue = d[0x60 / 4];
    u32 redDown, redUp, greenDown, greenUp, blueDown, blueUp;
    MaskShifts(red, redDown, redUp);
    MaskShifts(green, greenDown, greenUp);
    MaskShifts(blue, blueDown, blueUp);
    if (void *const old = at<void *>(image, 0xc)) GameHeap_Free(old);
    u8 *out = static_cast<u8 *>(GameHeap_Alloc(0x4b000));
    at<u8 *>(image, 0xc) = out;
    if (out != nullptr) {
        at<u32>(image, 0) = 0x140;
        at<u32>(image, 4) = 0xf0;
        at<u32>(image, 8) = 4;
    }
    const u32 xstep = ((d[0xc / 4] << 16) + 0x8000) / 320;
    const u32 ystep = ((d[8 / 4] << 16) + 0x8000) / 240;
    u8 *const rows = static_cast<u8 *>(GameHeap_Alloc(((ystep >> 16) + 1) * 0x500));
    if (rows == nullptr) return 0;
    u8 *const xcounts = static_cast<u8 *>(GameHeap_Alloc(0x140));
    u8 *const ycounts = static_cast<u8 *>(GameHeap_Alloc(0xf0));
    if (xcounts == nullptr || ycounts == nullptr) return 0;  // (the blocks taken kept, sic)
    u32 position = 0;
    for (u32 i = 0; i < 0x140; ++i) {
        const u32 before = position;
        position += xstep;
        xcounts[i] = (u8)((position >> 16) - (before >> 16));
    }
    position = 0;
    for (u32 j = 0; j < 0xf0; ++j) {
        const u32 before = position;
        position += ystep;
        ycounts[j] = (u8)((position >> 16) - (before >> 16));
    }
    const u32 bits = d[0x54 / 4];
    const u8 *row = reinterpret_cast<const u8 *>((uintptr_t)d[0x24 / 4]);
    if (bits == 16) {
        const u32 step = (d[0x10 / 4] >> 1) * 2;
        const u16 *source = reinterpret_cast<const u16 *>(row);
        u32 first = 0;  // (byte 0: never reset, sic)
        for (u32 j = 0; j < 0xf0; ++j) {
            const u32 yc = ycounts[j];
            u8 *band = rows;
            if (yc != 0) {
                for (u32 k = yc; k != 0; --k) {
                    for (u32 i = 0; i < 0x140; ++i) {
                        const u32 xc = xcounts[i];
                        u32 sumR = 0, sumG = 0, sumB = 0;
                        if (xc != 0) {
                            first += xc * 255;
                            for (u32 n = xc; n != 0; --n) {
                                const u32 pixel = *source++;
                                sumR += ((red & pixel) >> redDown) << redUp;
                                sumG += ((green & pixel) >> greenDown) << greenUp;
                                sumB += ((blue & pixel) >> blueDown) << blueUp;
                            }
                        }
                        first = (first & 0xffff) / xc;  // (a count of 0 divides by 0, sic)
                        band[0] = (u8)first;
                        band[1] = (u8)((sumR & 0xffff) / xc);
                        band[2] = (u8)((sumG & 0xffff) / xc);
                        band[3] = (u8)((sumB & 0xffff) / xc);
                        band += 4;
                    }
                    row += step;
                    source = reinterpret_cast<const u16 *>(row);
                }
            }
            first = ShrinkBandDown(rows, yc, out);
        }
    } else if (bits == 32) {
        const u8 *source = row;
        for (u32 j = 0; j < 0xf0; ++j) {
            const u32 yc = ycounts[j];
            u8 *band = rows;
            if (yc != 0) {
                for (u32 k = yc; k != 0; --k) {
                    for (u32 i = 0; i < 0x140; ++i) {
                        const u32 xc = xcounts[i];
                        u32 sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
                        for (u32 n = xc; n != 0; --n) {
                            sum0 += source[0];
                            sum1 += source[1];
                            sum2 += source[2];
                            sum3 += source[3];
                            source += 4;
                        }
                        band[0] = (u8)((sum3 & 0xffff) / xc);
                        band[1] = (u8)((sum2 & 0xffff) / xc);
                        band[2] = (u8)((sum1 & 0xffff) / xc);
                        band[3] = (u8)((sum0 & 0xffff) / xc);
                        band += 4;
                    }
                    row += d[0x10 / 4];
                    source = row;
                }
            }
            ShrinkBandDown(rows, yc, out);
        }
    } else {
        return bits == 24 ? 0 : 1;
    }
    GameHeap_Free(ycounts);
    GameHeap_Free(xcounts);
    GameHeap_Free(rows);
    return 1;
}

u8 __fastcall Image_ShrinkToImage(const u8 *image, void *, u8 *dest) {
    const s32 width = at<s32>(dest, 0), height = at<s32>(dest, 4);
    const u32 xstep = (u32)((s32)((at<u32>(image, 0) << 16) + 0x8000) / width);  // (signed, sic)
    const u32 ystep = (u32)((s32)((at<u32>(image, 4) << 16) + 0x8000) / height);
    const u8 *source = at<const u8 *>(image, 0xc);
    u8 *out = at<u8 *>(dest, 0xc);
    u8 *const rows = static_cast<u8 *>(GameHeap_Alloc((((ystep >> 16) + 1) * (u32)width) << 2));
    if (rows == nullptr) return 0;
    u8 *const xcounts = static_cast<u8 *>(GameHeap_Alloc((u32)width));
    u8 *const ycounts = static_cast<u8 *>(GameHeap_Alloc((u32)height));
    if (xcounts == nullptr || ycounts == nullptr) return 0;  // (the blocks taken kept, sic)
    u32 position = 0;
    for (s32 i = 0; i < width; ++i) {
        const u32 before = position;
        position += xstep;
        xcounts[i] = (u8)((position >> 16) - (before >> 16));
    }
    position = 0;
    for (s32 j = 0; j < height; ++j) {
        const u32 before = position;
        position += ystep;
        ycounts[j] = (u8)((position >> 16) - (before >> 16));
    }
    for (s32 j = 0; j < height; ++j) {
        const u32 yc = ycounts[j];
        u8 *band = rows;
        for (u32 k = yc; k != 0; --k) {
            for (s32 i = 0; i < width; ++i) {
                const u32 xc = xcounts[i];
                u32 sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
                for (u32 n = xc; n != 0; --n) {
                    sum0 += source[0];
                    sum1 += source[1];
                    sum2 += source[2];
                    sum3 += source[3];
                    source += 4;
                }
                band[0] = (u8)((sum0 & 0xffff) / xc);  // (a count of 0 divides by 0, sic)
                band[1] = (u8)((sum1 & 0xffff) / xc);
                band[2] = (u8)((sum2 & 0xffff) / xc);
                band[3] = (u8)((sum3 & 0xffff) / xc);
                band += 4;
            }
        }
        for (s32 i = 0; i < width; ++i) {
            const u8 *column = rows + i * 4;
            u32 sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
            for (u32 k = yc; k != 0; --k) {
                sum0 += column[0];
                sum1 += column[1];
                sum2 += column[2];
                sum3 += column[3];
                column += (u32)width * 4;
            }
            out[0] = (u8)((sum0 & 0xffff) / yc);
            out[1] = (u8)((sum1 & 0xffff) / yc);
            out[2] = (u8)((sum2 & 0xffff) / yc);
            out[3] = (u8)((sum3 & 0xffff) / yc);
            out += 4;
        }
    }
    GameHeap_Free(ycounts);
    GameHeap_Free(xcounts);
    GameHeap_Free(rows);
    return 1;
}
}  // extern "C"

namespace {
AddrCalls g_calls;
}

void Image_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __cdecl Surface_LoadTga(const char *name, u8 *surface, u32 *frames) {
    TgaCodec codec;   // (on the stack: the whole object, as the reader writes up to its header at +0x430)
    SurfaceInfo_Ctor(&codec.info);
    codec.vtable = reinterpret_cast<void *const *>((uintptr_t)0x006fefa8);
    SurfaceInfo info;
    SurfaceInfo_Ctor(&info);
    s32 status;
    if (*reinterpret_cast<s32 *>((uintptr_t)g_calls.Th(0x005fa8a0, (u32)(uintptr_t)&codec, &status, name, 0u, &info)) < 0) return 0;   // ImageCodec_InfoFromFile
    g_calls.Cd(0x005f9070, &status, info.width, info.height, 0x800u, surface);   // Surface_Create
    void *bits;   // (unread: Surface_GetInfo gives them too)
    if (*reinterpret_cast<s32 *>((uintptr_t)g_calls.Cd(0x005f9550, &status, surface, &bits)) < 0) return 0;   // Surface_Lock
    g_calls.Cd(0x005fad40, &info, surface);   // Surface_GetInfo
    if (*reinterpret_cast<s32 *>((uintptr_t)g_calls.Th(0x005faa00, (u32)(uintptr_t)&codec, &status, &info, name, 0u, 0u)) < 0) {   // ImageCodec_LoadFromFile
        g_calls.Cd(0x005f9130, &status, surface);   // Surface_Release
        return 0;
    }
    g_calls.Cd(0x005f9600, surface);   // Surface_Unlock
    *frames = info.width >> 5;
    return 1;
}

u8 __fastcall Image_LoadFile(u8 *image, void *, const char *directory, const char *name) {
    TgaCodec codec;   // (on the stack: the whole object, as the reader writes up to its header at +0x430)
    SurfaceInfo_Ctor(&codec.info);
    codec.vtable = reinterpret_cast<void *const *>((uintptr_t)0x006fefa8);
    SurfaceInfo info;
    SurfaceInfo_Ctor(&info);
    char path[0x100];
    char *end = path;
    while ((*end = *directory++)) ++end;
    while ((*end++ = *name++)) {}
    s32 status;
    if (*reinterpret_cast<s32 *>((uintptr_t)g_calls.Th(0x005fa8a0, (u32)(uintptr_t)&codec, &status, path, 0u, &info)) < 0) return 0;   // ImageCodec_InfoFromFile
    g_calls.Th(0x005fa6f0, (u32)(uintptr_t)&info, &status);   // SurfaceInfo_Alloc
    if (*reinterpret_cast<s32 *>((uintptr_t)g_calls.Th(0x005faa00, (u32)(uintptr_t)&codec, &status, &info, path, 0u, 0u)) < 0) {   // ImageCodec_LoadFromFile
        g_calls.Th(0x005fa740, (u32)(uintptr_t)&info);   // SurfaceInfo_Free
        return 0;
    }
    const bool single = info.paletted == 1;
    u8 *&block = *reinterpret_cast<u8 **>(image + 0xc);
    if (block) GameHeap_Free(block);
    const u32 size = single ? info.width * info.height : info.width * info.height * 4;
    u8 *const pixels = static_cast<u8 *>(GameHeap_Alloc(size));
    block = pixels;
    if (!pixels) return 0;   // sic: the bits stay allocated
    if (single) {
        std::memcpy(pixels, info.bits, size);
    } else {
        const u8 *from = static_cast<const u8 *>(info.bits);
        u8 *to = pixels;
        for (u32 y = 0; y < info.height; ++y)
            for (u32 x = 0; x < info.width; ++x, from += 4, to += 4) {
                to[0] = from[3];
                to[1] = from[2];
                to[2] = from[1];
                to[3] = from[0];
            }
    }
    *reinterpret_cast<u32 *>(image) = info.width;
    *reinterpret_cast<u32 *>(image + 4) = info.height;
    *reinterpret_cast<u32 *>(image + 8) = single ? 1 : 4;
    g_calls.Th(0x005fa740, (u32)(uintptr_t)&info);
    return 1;
}
}  // extern "C"
