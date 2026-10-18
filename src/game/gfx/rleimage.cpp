#include "rleimage.h"

#include <cstdint>
#include "../core/gameheap.h"
#include "../core/vwrap.h"

namespace {

template <typename T> T field(const void *p, s32 offset) { return *reinterpret_cast<const T *>((uintptr_t)p + (uintptr_t)(intptr_t)offset); }

// The original's rep movsd / rep movsb: dwords forward, then the bytes left.
void CopyForward(u8 *to, const u8 *from, u32 bytes) {
    for (u32 k = bytes >> 2; k != 0; k--) {
        *reinterpret_cast<u32 *>(to) = *reinterpret_cast<const u32 *>(from);
        to += 4;
        from += 4;
    }
    for (u32 k = bytes & 3; k != 0; k--) *to++ = *from++;
}

// One source row's codes decoded to out (u16 or u32 palette entries) until the width is reached; returns the end of what was written.
template <typename Pixel> u8 *DecodeRow(const u8 *&codes, const Pixel *palette, s32 width, u8 *out) {
    if (width <= 0) return out;
    s32 x = 0;
    do {
        s32 n = (s8)*codes++;
        if (n > 0) {
            for (s32 k = n; k != 0; k--) {
                *reinterpret_cast<Pixel *>(out) = palette[*codes++];
                out += sizeof(Pixel);
            }
        } else {
            const Pixel value = palette[*codes++];
            n = -n;
            for (s32 k = n; k != 0; k--) {
                *reinterpret_cast<Pixel *>(out) = value;
                out += sizeof(Pixel);
            }
        }
        x += n;
    } while (x < width);
    return out;
}

template <typename Pixel> u8 *SampleRow(const u8 *row, u32 count, s32 step, u8 *out) {
    u32 position = 0;
    for (; count != 0; count--) {
        *reinterpret_cast<Pixel *>(out) = reinterpret_cast<const Pixel *>(row)[(s32)position >> 16];
        out += sizeof(Pixel);
        position += (u32)step;
    }
    return out;
}

}  // namespace

extern "C" {

u32 __fastcall RleImage_DrawScaled(const u8 *image, void *, const u32 *dest) {
    const u32 size = reinterpret_cast<const u32 *>((uintptr_t)0x00701158)[*reinterpret_cast<const u32 *>((uintptr_t)0x008bd554)];
    const s32 width = field<s32>(image, -0xc);
    const s32 height = field<s32>(image, -8);
    const u8 *codes = field<const u8 *>(image, -0x14);
    const u8 *const palette = field<const u8 *>(image, -0x10);
    u8 *out = reinterpret_cast<u8 *>((uintptr_t)dest[0]);
    const s32 xstep = (s32)((u32)width << 16) / (s32)dest[1];
    const s32 ystep = (s32)((u32)height << 16) / (s32)dest[2];
    const bool scaled = (u32)width != dest[1];
    const u32 pad = dest[3] != 0 ? dest[3] - size * dest[1] : 0;
    u8 row[0x200];
    u32 state = 1;
    u32 y = 0;
    if (height <= 0) return 1;
    for (s32 n = height; n != 0; n--) {
        if (state == 0) {
            codes += (s8)codes[0] + 1;
        } else if (state == 2) {
            CopyForward(out, out - dest[3], size * dest[1]);
            out += dest[3];
            y += (u32)ystep;
        } else {
            codes++;
            u8 *const to = scaled ? row : out;
            u8 *end = size == 2 ? DecodeRow(codes, reinterpret_cast<const u16 *>(palette), width, to)
                                : DecodeRow(codes, reinterpret_cast<const u32 *>(palette), width, to);
            if (scaled) end = size == 2 ? SampleRow<u16>(row, dest[1], xstep, out) : SampleRow<u32>(row, dest[1], xstep, out);
            out = end + pad;
            y += (u32)ystep;
        }
        if ((s32)y >= 0x20000) {
            state = 0;
            y -= 0x10000;
        } else {
            state = (y & 0x10000) != 0 ? 1 : 2;
            y &= 0xffff;
        }
    }
    return 1;
}
}

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall RleImage_ReadCodes(u8 *image, void *, u8 *stream, u32 size) {
    const u32 block = TrackedGlobal_Alloc(size);
    at<u32>(image, 4) = block;
    if (!block) return 0;
    Object_CallSlot8Product(block, 1, size, stream);
    return 1;
}

u8 *__fastcall RleImage_DeletingDtor(u8 *part, void *, u32 flags) {
    u8 *image = part - 0x18;
    at<u32>(image, at<u32>(at<u8 *>(image, 0), 4)) = 0x00701168;
    const u32 base = at<u32>(at<u8 *>(image, 0), 4);
    at<u32>(image, base - 4) = base - 0x18;
    if (at<u32>(image, 4)) {
        TrackedGlobal_Free(at<u32>(image, 4));
        at<u32>(image, 4) = 0;
    }
    at<u32>(image, 0x10) = 0;
    at<u32>(image, 0xc) = 0;
    at<u32>(image, 0x18) = 0x00701178;
    if (flags & 1) GameHeap_Free(image);
    return image;
}
}
