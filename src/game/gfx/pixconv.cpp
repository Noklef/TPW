// The pixel converters: see pixconv.h.
#include "pixconv.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const PixConvEnv kGameEnv = {
    at<void(__cdecl *)(void *)>(0x0067acb0),
};
const PixConvEnv *g_env = &kGameEnv;

void *const *const kBaseVtable = at<void *const *>(0x007039c0);

// A mask's top set bit and the length of the run of set bits under it (mask != 0).
void run(u32 mask, s32 *top, s32 *width) {
    u32 bit = 0x80000000u;
    s32 i = 31;
    while (!(mask & bit)) {
        bit >>= 1;
        --i;
    }
    *top = i;
    while (mask & bit) {
        bit >>= 1;
        --i;
    }
    *width = *top - i;
}

u32 pixel(const PaletteConverter *c, u8 index) {
    const u32 p = c->palette[index];
    u32 out = c->alpha;
    for (int k = 0; k < 3; ++k) out |= ((p & c->mask[k]) >> (c->right[k] & 31)) << (c->left[k] & 31);
    return out;
}

void put(u8 *dst, u32 px, u32 bytes) {
    if (bytes == 3) {
        dst[0] = static_cast<u8>(px);
        dst[1] = static_cast<u8>(px >> 8);
        dst[2] = static_cast<u8>(px >> 16);
    } else {
        std::memcpy(dst, &px, bytes);
    }
}

u32 rotr(u32 v, u32 n) {
    n &= 31;
    return n ? v >> n | v << (32 - n) : v;
}

u32 rgb(const RgbConverter *c, const u8 *src) {
    u32 s;
    std::memcpy(&s, src, 4);
    u32 out = c->fill;
    for (int k = 0; k < 4; ++k) {
        const u32 v = s & c->sourceMask[k];
        u32 t = rotr(v, c->rotate[k]);
        if (c->widens) t |= rotr(v, c->rotate[k] + c->width[k]);
        out |= t & c->targetMask[k];
    }
    return out;
}

}  // namespace

void PixConv_SetEnv(const PixConvEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void *__fastcall PixelConverter_DeletingDtor(void *c, void *, u32 flags) {
    *static_cast<void *const **>(c) = kBaseVtable;
    if (flags & 1) g_env->free(c);
    return c;
}

void __fastcall PixelConverter_Dtor(void *c) { *static_cast<void *const **>(c) = kBaseVtable; }

void *__fastcall PixelConverter_DeletingDtor2(void *c, void *, u32 flags) {
    PixelConverter_Dtor(c);
    if (flags & 1) g_env->free(c);
    return c;
}

void __fastcall PaletteConverter_Init(PaletteConverter *c, void *, const u32 *palette, const u32 *format) {
    std::memcpy(c->palette, palette, sizeof c->palette);
    c->mask[0] = 0xff;
    c->mask[1] = 0xff00;
    c->bytes = (format[4] + 7) >> 3;
    c->mask[2] = 0xff0000;
    for (int k = 0; k < 3; ++k) Mask_Shifts(c->mask[k], format[k], &c->right[k], &c->left[k]);
    c->alpha = format[3];
}

void __fastcall PaletteConverter_Convert(PaletteConverter *c, void *, u8 *dst, const u8 *src, u32 count) {
    const u32 bytes = c->bytes;
    if (bytes < 1 || bytes > 4) return;
    for (u32 i = 0; i < count; ++i, dst += bytes) put(dst, pixel(c, src[i]), bytes);
}

void __fastcall PaletteConverter_ConvertBack(PaletteConverter *c, void *, u8 *dst, const u8 *src, u32 count) {
    const u32 bytes = c->bytes;
    if (bytes < 1 || bytes > 4) return;
    u8 *out = dst + bytes * (count - 1);
    for (u32 i = count; i; --i, out -= bytes) put(out, pixel(c, src[i - 1]), bytes);
}

void __stdcall Mask_Shifts(u32 source, u32 target, u32 *right, u32 *left) {
    if (!source) {
        *right = 0;
        *left = 0;
        return;
    }
    s32 top, width;
    run(source, &top, &width);
    if (!target) {
        *left = static_cast<u32>(width - top + 31);
        *right = 0;
        if (*left >= 32) {
            *left = 0;
            *right = 31;
        }
        return;
    }
    s32 targetTop, targetWidth;
    run(target, &targetTop, &targetWidth);
    const s32 kept = width < targetWidth ? width : targetWidth;
    *right = static_cast<u32>(top - kept + 1);
    *left = static_cast<u32>(targetTop - targetWidth + 1);
}

void __fastcall RgbConverter_Init(RgbConverter *c, void *, const u32 *source, const u32 *target) {
    c->targetBytes = (target[4] + 7) >> 3;
    c->sourceBytes = (source[4] + 7) >> 3;
    for (int k = 0; k < 4; ++k) c->sourceMask[k] = source[k];
    for (int k = 0; k < 4; ++k) c->targetMask[k] = target[k];
    c->widens = 0;
    for (int k = 0; k < 4; ++k)
        RgbConverter_ChannelShift(c, nullptr, source[k], target[k], &c->rotate[k], &c->width[k]);
    c->fill = 0;
    if (!source[3]) c->fill = target[3];
}

void __fastcall RgbConverter_Convert(RgbConverter *c, void *, u8 *dst, const u8 *src, u32 count) {
    const u32 bytes = c->targetBytes;
    if (bytes < 1 || bytes > 4) return;
    for (u32 i = 0; i < count; ++i, dst += bytes, src += c->sourceBytes) put(dst, rgb(c, src), bytes);
}

void __fastcall RgbConverter_ConvertBack(RgbConverter *c, void *, u8 *dst, const u8 *src, u32 count) {
    const u32 bytes = c->targetBytes;
    if (bytes < 1 || bytes > 4) return;
    dst += bytes * (count - 1);
    src += c->sourceBytes * (count - 1);
    for (u32 i = count; i; --i, dst -= bytes, src -= c->sourceBytes) put(dst, rgb(c, src), bytes);
}

void __fastcall RgbConverter_ChannelShift(RgbConverter *c, void *, u32 source, u32 target, u32 *rotate,
                                          s32 *width) {
    if (!source || !target) {
        *rotate = 0;
        return;
    }
    s32 top, targetTop, targetWidth;
    run(source, &top, width);
    run(target, &targetTop, &targetWidth);
    if (*width < targetWidth && !c->widens) c->widens = 1;
    const s32 r = top - targetTop;
    *rotate = static_cast<u32>(r < 0 ? r + 32 : r);
}

}  // extern "C"
