#include "small53.h"

#include <cstdint>

#include "gameheap.h"

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

}  // namespace

extern "C" {

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

u32 __cdecl Image_Dilate(u32 *image, s32 radius) { return Filter<true>(image, radius); }

u32 __cdecl Image_Erode(u32 *image, s32 radius) { return Filter<false>(image, radius); }

}  // extern "C"
