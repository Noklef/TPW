// The pixel converters: see pixconv.h.
#include "pixconv.h"

#include "../core/vcall.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const PixConvEnv kGameEnv = {
    at<void(__cdecl *)(void *)>(0x0067acb0),
    at<void *(__stdcall *)(u32)>(0x00642860),
    at<s32(__stdcall *)(void *)>(0x00642880),
    at<void *(__fastcall *)(void *, void *)>(0x00642150),
    at<void(__fastcall *)(void *, void *, u32, u32, const u32 *, u32, u32 *, u8 *)>(0x00642190),
    at<void(__fastcall *)(void *, void *)>(0x00642180),
    at<const u32 *>(0x00faa268),
    at<const u32 *>(0x00faa280),
    at<void *(__cdecl *)(u32)>(0x0067acc0),
};
const PixConvEnv *g_env = &kGameEnv;

void *const *const kBaseVtable = at<void *const *>(0x007039c0);
void *const *const kIndexVtable = at<void *const *>(0x00703fa8);
void *const *const kRgb16Vtable = at<void *const *>(0x00703fb8);
void *const *const kRgbVtable = at<void *const *>(0x007039b0);
void *const *const kPaletteVtable = at<void *const *>(0x007039a0);

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

u8 indexed(IndexConverter *c, const u8 *src) {
    u32 s;
    std::memcpy(&s, src, 4);
    u32 v[3];
    for (int k = 0; k < 3; ++k) v[k] = ((s & c->mask[k]) >> (c->right[k] & 31)) << (c->left[k] & 31);
    if (c->table) return c->table[v[0] | v[1] | v[2]];
    return IndexConverter_Nearest(c, nullptr, v[0] & 0xff, v[1] & 0xff, v[2] & 0xff);
}

bool sameFormat(const u32 *a, const u32 *b) {
    for (int k = 0; k < 5; ++k)
        if (a[k] != b[k]) return false;
    return !a[5] == !b[5];
}

// A target field's rotation (from 8) and the top bits of a byte it keeps.
void field(u32 mask, u32 *rotate, u32 *keep) {
    if (!mask) return;
    for (; !(mask & 1); mask >>= 1) ++*rotate;
    for (; mask; mask >>= 1, ++*rotate) *keep = (*keep >> 1) + 0x80;
}

u32 rotl(u32 v, u32 n) {
    n &= 31;
    return n ? v << n | v >> (32 - n) : v;
}

// Drops the converter for a new one of kind (allocated, or null).
void *replace(PixelConversion *c, u32 kind, u32 size) {
    if (c->converter) vcall<void *>(c->converter, 0, 1u);
    c->converter = g_env->allocate(size);
    c->kind = kind;
    return c->converter;
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

IndexConverter *__fastcall IndexConverter_Ctor(IndexConverter *c, void *, const u32 *format, const u32 *palette) {
    c->table = nullptr;
    c->built = nullptr;
    c->vtable = kIndexVtable;
    IndexConverter_Init(c, nullptr, format, palette);
    return c;
}

void __fastcall IndexConverter_Dtor(IndexConverter *c) {
    c->vtable = kIndexVtable;
    IndexConverter_FreeTable(c);
    c->vtable = kBaseVtable;
}

void *__fastcall IndexConverter_DeletingDtor(IndexConverter *c, void *, u32 flags) {
    IndexConverter_Dtor(c);
    if (flags & 1) g_env->free(c);
    return c;
}

void __fastcall IndexConverter_Init(IndexConverter *c, void *, const u32 *format, const u32 *palette) {
    std::memcpy(c->palette, palette, sizeof c->palette);
    c->bits = 5;
    c->levels = 0x20;
    c->bytes = (format[4] + 7) >> 3;
    if (!c->table) c->table = static_cast<u8 *>(g_env->alloc(0x8000));
    if (c->table && palette != c->built) {
        u8 map[0x58];
        g_env->mapInit(map, nullptr);
        u32 *work = static_cast<u32 *>(g_env->alloc((1u << (c->bits * 3 & 31)) << 2));
        g_env->mapBuild(map, nullptr, 0, 0x100, palette, c->bits, work, c->table);
        g_env->release(work);
        g_env->mapDone(map, nullptr);
    }
    c->built = palette;
    for (int k = 0; k < 3; ++k) c->mask[k] = format[k];
    u32 field[3] = {0xff, 0xff, 0xff};
    if (c->table) {
        field[2] = c->levels - 1;
        field[1] = field[2] << (c->bits & 31);
        field[0] = field[1] << (c->bits & 31);
    }
    for (int k = 0; k < 3; ++k) Mask_Shifts(c->mask[k], field[k], &c->right[k], &c->left[k]);
}

void __fastcall IndexConverter_FreeTable(IndexConverter *c) {
    if (c->table) g_env->release(c->table);
}

void __fastcall IndexConverter_Convert(IndexConverter *c, void *, u8 *dst, const u8 *src, u32 count) {
    for (u32 i = 0; i < count; ++i, src += c->bytes) dst[i] = indexed(c, src);
}

void __fastcall IndexConverter_ConvertBack(IndexConverter *c, void *, u8 *dst, const u8 *src, u32 count) {
    src += c->bytes * (count - 1);
    for (u32 i = count; i; --i, src -= c->bytes) dst[i - 1] = indexed(c, src);
}

u8 __fastcall IndexConverter_Nearest(IndexConverter *c, void *, u32 red, u32 green, u32 blue) {
    const u8 *p = reinterpret_cast<const u8 *>(c->palette);
    u32 best = 0x7fffffff;
    u8 found = static_cast<u8>(red);
    for (u32 i = 0; i < 0x100 && best; ++i, p += 4) {
        const s32 r = static_cast<s32>(red & 0xff) - p[0];
        const s32 g = static_cast<s32>(green & 0xff) - p[1];
        const s32 b = static_cast<s32>(blue & 0xff) - p[2];
        const u32 d = static_cast<u32>(r * r + g * g + b * b);
        if (d < best) {
            best = d;
            found = static_cast<u8>(i);
        }
    }
    return found;
}

Rgb16Converter *__fastcall Rgb16Converter_Ctor(Rgb16Converter *c, void *, const u32 *source, const u32 *target) {
    c->vtable = kRgb16Vtable;
    Rgb16Converter_Init(c, nullptr, source, target);
    return c;
}

void __fastcall Rgb16Converter_Init(Rgb16Converter *c, void *, const u32 *source, const u32 *target) {
    if (!sameFormat(source, g_env->rgb888) && !sameFormat(source, g_env->argb8888)) return;
    if (((target[4] + 7) & ~7u) != 0x10) return;
    c->fill = target[3];
    for (int k = 0; k < 4; ++k) {
        c->rotate[k] = 8;
        c->keep[k] = 0;
    }
    for (int k = 0; k < 3; ++k) field(target[k], &c->rotate[k], &c->keep[k]);
    if (sameFormat(source, g_env->rgb888)) {
        c->from24 = 1;
        c->fill += c->fill << 16;
    } else {
        field(c->fill, &c->rotate[3], &c->keep[3]);
        c->fill = 0;
        c->from24 = 0;
    }
    for (int k = 0; k < 4; ++k) c->keep[k] += c->keep[k] << 16;
}

void __fastcall Rgb16Converter_Dtor(void *c) { *static_cast<void *const **>(c) = kBaseVtable; }

void *__fastcall Rgb16Converter_DeletingDtor(void *c, void *, u32 flags) {
    Rgb16Converter_Dtor(c);
    if (flags & 1) g_env->free(c);
    return c;
}

void __fastcall Rgb16Converter_Convert(Rgb16Converter *c, void *, u8 *dst, const u8 *src, s32 count) {
    static const int kChannel[4] = {2, 1, 0, 3};  // the source bytes: blue, green, red, alpha
    const u32 step = c->from24 ? 3 : 4;
    const u32 fill = c->from24 ? c->fill : 0;
    u32 pairs = static_cast<u32>(count / 2);
    do {
        u32 px = fill;
        for (u32 k = 0; k < step; ++k) {
            const int ch = kChannel[k];
            px |= rotl(((static_cast<u32>(src[k]) << 16) + src[step + k]) & c->keep[ch], c->rotate[ch]);
        }
        std::memcpy(dst, &px, 4);
        dst += 4;
        src += 2 * step;
    } while (--pairs);
}

PixelConversion *__fastcall PixelConversion_Ctor(PixelConversion *c) {
    c->converter = nullptr;
    c->kind = 4;
    return c;
}

void __fastcall PixelConversion_Dtor(PixelConversion *c) {
    if (!c->converter) return;
    vcall<void *>(c->converter, 0, 1u);
    c->converter = nullptr;
    c->kind = 4;
}

void __fastcall PixelConversion_Setup(PixelConversion *c, void *, const u32 *target, const u32 *source,
                                      const u32 *palette, u32 flags) {
    c->sourceBytes = (source[4] + 7) >> 3;
    const u32 targetBytes = (target[4] + 7) >> 3;
    c->targetBytes = targetBytes;
    if ((sameFormat(source, g_env->argb8888) || sameFormat(source, g_env->rgb888)) && !(flags & 1) &&
        targetBytes == 2) {
        if (c->kind == 0) {
            Rgb16Converter_Init(static_cast<Rgb16Converter *>(c->converter), nullptr, source, target);
        } else if (void *p = replace(c, 0, 0x2c)) {
            c->converter = Rgb16Converter_Ctor(static_cast<Rgb16Converter *>(p), nullptr, source, target);
        }
    } else if (!source[5] && !target[5]) {
        if (c->kind == 1) {
            RgbConverter_Init(static_cast<RgbConverter *>(c->converter), nullptr, source, target);
        } else if (void *p = replace(c, 1, 0x54)) {
            static_cast<RgbConverter *>(p)->vtable = kRgbVtable;
            RgbConverter_Init(static_cast<RgbConverter *>(p), nullptr, source, target);
        }
    } else if (source[5] && !target[5]) {
        if (!palette) return;
        if (c->kind == 2) {
            PaletteConverter_Init(static_cast<PaletteConverter *>(c->converter), nullptr, palette, target);
        } else if (void *p = replace(c, 2, 0x430)) {
            static_cast<PaletteConverter *>(p)->vtable = kPaletteVtable;
            PaletteConverter_Init(static_cast<PaletteConverter *>(p), nullptr, palette, target);
        }
    } else if (!source[5] && target[5]) {
        if (!palette) return;
        if (c->kind == 3) {
            IndexConverter_Init(static_cast<IndexConverter *>(c->converter), nullptr, source, palette);
        } else if (void *p = replace(c, 3, 0x43c)) {
            c->converter = IndexConverter_Ctor(static_cast<IndexConverter *>(p), nullptr, source, palette);
        }
    }
}

void __fastcall PixelConversion_Row(PixelConversion *c, void *, u8 *dst, const u8 *src, u32 count) {
    vcall<void>(c->converter, c->sourceBytes < c->targetBytes ? 8 : 4, dst, src, count);
}

void __fastcall PixelConversion_Rect(PixelConversion *c, void *, u8 *dst, const u8 *src, s32 dstPitch,
                                     s32 srcPitch, u32 rows, u32 width) {
    if (!dst) dst = const_cast<u8 *>(src);
    const bool back = dst == src && c->targetBytes < c->sourceBytes;
    for (; rows; --rows, src += srcPitch, dst += dstPitch) vcall<void>(c->converter, back ? 8 : 4, dst, src, width);
}

}  // extern "C"
