// The glyph renderers (see glyphrender.h).
#include "glyphrender.h"
#include <cstdint>
#include "../core/gameheap.h"
#include "../core/thunks.h"
#include "../core/vtables.h"

namespace {
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// The walk every span writer but GlyphSolid_Span565 shares: plot(pixel, colour, coverage) for each pixel of the glyph's rectangle.
template <typename P, typename Plot> s32 *Span(s32 *status, const u8 *surface, const u8 *span, Plot plot) {
    const u32 colour = at<u32>(at<const u8 *>(span, 0x10), 0x44);
    const u32 alpha = colour >> 24;
    if (alpha) {
        const s32 *point = at<const s32 *>(span, 0);
        const s32 *rect = at<const s32 *>(span, 0xc);
        u32 row = (u32)point[1] * at<u32>(surface, 0x24) + at<u32>(surface, 0x18) + (u32)point[0] * sizeof(P);
        const u32 length = at<u8>(span, 0x14);
        const u32 first = (u32)rect[1] * length + (u32)rect[0];
        bool low = (first & 1) != 0;   // the nibble is the low one of its byte (the high one comes first)
        const u8 *nibbles = at<const u8 *>(span, 4) + (first >> 1);
        const u32 skip = (u32)rect[0] - (u32)rect[2] + length;
        for (u32 rows = (u32)rect[3] - (u32)rect[1]; rows; --rows) {
            P *pixel = reinterpret_cast<P *>((uintptr_t)row);
            row += at<u32>(surface, 0x24);
            const s32 *r = at<const s32 *>(span, 0xc);
            for (u32 cols = (u32)r[2] - (u32)r[0]; cols; --cols) {
                const u32 nibble = low ? (*nibbles & 0xf) : (u32)(*nibbles >> 4);
                if (low) ++nibbles;
                low = !low;
                plot(pixel++, colour, alpha == 0xff ? nibble : (u32)((s32)(nibble * alpha) / 255));
            }
            if (skip) {
                if (skip & 1) {
                    nibbles += low ? (skip + 1) >> 1 : skip >> 1;
                    low = !low;
                } else {
                    nibbles += skip >> 1;
                }
            }
        }
    }
    *status = 0;
    return status;
}

// a % b in one DIV, as the originals': it faults (sic) for b == 0 even where the remainder goes unused.
u32 Remainder(u32 a, u32 b) {
    u32 r;
    __asm {
        mov eax, a
        xor edx, edx
        div b
        mov r, edx
    }
    return r;
}

// The walk the patterned span writers share: the pattern's pixel (a dword) for each pixel of the glyph's rectangle, the pattern tiled from
// its origin; plot(pixel, pattern pixel, coverage) with the coverage the glyph's nibble (Glyph) or 0 (no glyph walked).
template <typename P, bool Glyph, typename Plot> s32 *PatternSpan(s32 *status, const u8 *surface, const u8 *span, Plot plot) {
    const u8 *style = at<const u8 *>(span, 0x10);
    const u32 *format = reinterpret_cast<const u32 *>(style + 0x14);
    const u32 *argb8888 = reinterpret_cast<const u32 *>((uintptr_t)0x00faa280);
    if (format[0] != argb8888[0] || format[1] != argb8888[1] || format[2] != argb8888[2] || format[3] != argb8888[3] ||
        format[4] != argb8888[4] || (format[5] == 0) != (argb8888[5] == 0)) {
        *status = -1;
        return status;
    }
    const u32 bits = format[6], width = format[7], height = format[8], pitch = format[9], originX = format[10], originY = format[11];
    const s32 *point = at<const s32 *>(span, 0);
    u32 row = (u32)point[1] * at<u32>(surface, 0x24) + at<u32>(surface, 0x18) + (u32)point[0] * sizeof(P);
    const s32 *rect = at<const s32 *>(span, 0xc);
    const u32 length = at<u8>(span, 0x14);
    const u32 first = (u32)rect[1] * length + (u32)rect[0];
    bool low = (first & 1) != 0;
    const u8 *nibbles = at<const u8 *>(span, 4) + (first >> 1);
    const u32 skip = (u32)rect[0] - (u32)rect[2] + length;
    u32 patternY = Remainder((u32)point[1] - originY, height);
    for (u32 rows = (u32)rect[3] - (u32)rect[1]; rows;) {
        u32 count = rows < height - patternY ? rows : height - patternY;
        rows -= count;
        u32 patternRow = patternY * pitch + bits;
        for (; count; --count) {
            const u32 nextRow = patternRow + pitch;
            const s32 *r = at<const s32 *>(span, 0xc);
            u32 cols = (u32)r[2] - (u32)r[0];
            u32 patternX = Remainder((u32)at<const s32 *>(span, 0)[0] - originX, width);
            P *pixel = reinterpret_cast<P *>((uintptr_t)row);
            while (cols) {
                u32 run = cols < width - patternX ? cols : width - patternX;
                cols -= run;
                const u32 *pattern = reinterpret_cast<const u32 *>((uintptr_t)(patternRow + patternX * 4));
                for (; run; --run) {
                    u32 nibble = 0;
                    if (Glyph) {
                        nibble = low ? (*nibbles & 0xf) : (u32)(*nibbles >> 4);
                        if (low) ++nibbles;
                        low = !low;
                    }
                    plot(pixel++, *pattern++, nibble);
                }
                patternX = 0;
            }
            row += at<u32>(surface, 0x24);
            if (Glyph && skip) {
                if (skip & 1) {
                    nibbles += low ? (skip + 1) >> 1 : skip >> 1;
                    low = !low;
                } else {
                    nibbles += skip >> 1;
                }
            }
            patternRow = nextRow;
        }
        patternY = 0;
    }
    *status = 0;
    return status;
}

// The pixel writers more than one span writer shares (see glyphrender.h).
void Blend1555(u16 *p, u32 colour, u32 coverage) {
    const s32 old = *p, c = (s32)coverage;
    const s32 g = ((s32)((colour >> 8 & 0xf8) << 2) - (old & 0x3e0)) * c / 15;
    const s32 r = ((s32)(colour >> 16 & 0xf8) - (old >> 7 & 0xf8)) * c / 15;
    const s32 b = ((s32)((colour & 0xff) >> 3) - (old & 0x1f)) * c / 15;
    *p = (u16)(((u32)(g + (old & 0xffe0)) & 0x3e0) | (((u32)r << 7) + (u32)(old & 0xfc00) & 0x7c00) | ((u32)(b + old) & 0x1f) |
               (u32)(old & 0x8000));
}

void Blend565(u16 *p, u32 colour, u32 coverage) {
    const s32 old = *p, c = (s32)coverage;
    const s32 g = ((s32)((colour >> 8 & 0xfc) << 3) - (old & 0x7e0)) * c / 15;
    const s32 r = ((s32)(colour >> 16 & 0xf8) - (old >> 8 & 0xf8)) * c / 15;
    const s32 b = ((s32)((colour & 0xff) >> 3) - (old & 0x1f)) * c / 15;
    *p = (u16)(((u32)(g + (old & 0xffe0)) & 0x7e0) | (((u32)r << 8) + (u32)(old & 0xf800) & 0xf800) | ((u32)(b + old) & 0x1f));
}

void Blend8888(u32 *p, u32 colour, u32 c) {
    const u32 old = *p;
    const u32 r = ((colour >> 16 & 0xff) << 16) - (old & 0xff0000);
    const u32 g = ((colour >> 8 & 0xff) << 8) - (old & 0xff00);
    const u32 b = (colour & 0xff) - (old & 0xff);
    *p = ((r * c / 15 + (old & 0xffff0000)) & 0xff0000) | ((g * c / 15 + (old & 0xffffff00)) & 0xff00) | ((b * c / 15 + old) & 0xff) |
         (old & 0xff000000);
}

void Solid4444(u16 *p, u32 colour, u32 c) {
    if (c) *p = (u16)(c << 12 | (colour >> 16 & 0xf0) << 4 | (colour >> 8 & 0xf0) | (colour & 0xff) >> 4);
}

void Solid8888(u32 *p, u32 colour, u32 c) {
    if (c) *p = c << 28 | c << 24 | (colour & 0xffffff);
}

u8 *Delete(u8 *object, u32 flags) {
    if (flags & 1) GameHeap_Free(object);
    return object;
}
}  // namespace

extern "C" {

u8 *__fastcall Vtbl70a148_DeletingDtor(u8 *object, void *, u32 flags) {
    Vtbl70a078_Set_6add30(object);
    return Delete(object, flags);
}

u8 *__fastcall Vtbl70a160_DeletingDtor(u8 *object, void *, u32 flags) {
    Wrap_Vtbl70a078_Set_6b4a50_6b0470(object);
    return Delete(object, flags);
}

u8 *__fastcall Vtbl70a210_DeletingDtor(u8 *object, void *, u32 flags) {
    Vtbl70a078_Set_6b4a50(object);
    return Delete(object, flags);
}

u8 *__fastcall GlyphRenderer_DeletingDtor(u8 *renderer, void *, u32 flags) {
    Vtbl70a258_Set_6b4a90(renderer);
    return Delete(renderer, flags);
}

u8 *__fastcall Vtbl70a258_DeletingDtor(u8 *object, void *, u32 flags) {
    at<u32>(object, 0) = 0x70a258;
    return Delete(object, flags);
}

u8 *__fastcall GlyphBlend_DeletingDtor(u8 *renderer, void *, u32 flags) {
    Wrap_Vtbl70a258_Set_6b4a90_6b0750(renderer);
    return Delete(renderer, flags);
}

u8 *__fastcall GlyphSolid_DeletingDtor(u8 *renderer, void *, u32 flags) {
    Wrap_Vtbl70a258_Set_6b4a90_6b1e20(renderer);
    return Delete(renderer, flags);
}

u8 *__fastcall GlyphPattern1_DeletingDtor(u8 *renderer, void *, u32 flags) {
    Wrap_Vtbl70a258_Set_6b4a90_6b2a50(renderer);
    return Delete(renderer, flags);
}

u8 *__fastcall GlyphPattern2_DeletingDtor(u8 *renderer, void *, u32 flags) {
    Wrap_Vtbl70a258_Set_6b4a90_6b3c40(renderer);
    return Delete(renderer, flags);
}

s32 *__fastcall GlyphBlend_Span1555(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return Span<u16>(status, surface, span, Blend1555);
}

s32 *__fastcall GlyphBlend_Span565(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return Span<u16>(status, surface, span, Blend565);
}

s32 *__fastcall GlyphBlend_Span4444(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return Span<u16>(status, surface, span, [](u16 *p, u32 colour, u32 coverage) {
        const s32 old = *p, c = (s32)coverage;
        const s32 r = ((s32)((colour >> 16 & 0xf0) << 4) - (old & 0xf00)) * c / 15;
        const s32 g = ((s32)(colour >> 8 & 0xf0) - (old & 0xf0)) * c / 15;
        const s32 b = ((s32)((colour & 0xff) >> 4) - (old & 0xf)) * c / 15;
        *p = (u16)(((u32)(r + (old & 0xff00)) & 0xf00) | ((u32)(g + (old & 0xfff0)) & 0xf0) | ((u32)(b + old) & 0xf) |
                   (u32)(old & 0xf000));
    });
}

s32 *__fastcall GlyphBlend_Span8888(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return Span<u32>(status, surface, span, Blend8888);
}

s32 *__fastcall GlyphBlend_SpanBgra(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return Span<u32>(status, surface, span, [](u32 *p, u32 colour, u32 c) {
        const u32 old = *p;
        const u32 g = ((colour >> 8 & 0xff) << 16) - (old & 0xff0000);
        const u32 r = ((colour >> 16 & 0xff) << 8) - (old & 0xff00);
        const u32 b = (colour << 24) - (old & 0xff000000);
        *p = ((g * c / 15 + (old & 0xffff0000)) & 0xff0000) | ((r * c / 15 + (old & 0xffffff00)) & 0xff00) |
             ((b * c / 15 + (old & 0xff000000)) & 0xff000000) | (old & 0xff);
    });
}

s32 *__fastcall GlyphSolid_Span1555(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return Span<u16>(status, surface, span, [](u16 *p, u32 colour, u32 c) {
        if (c & 8) *p = (u16)(0x8000 | (colour >> 16 & 0xf8) << 7 | (colour >> 8 & 0xf8) << 2 | (colour & 0xff) >> 3);
    });
}

s32 *__fastcall GlyphSolid_Span565(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    const u32 colour = at<u32>(at<const u8 *>(span, 0x10), 0x44);
    const s32 *point = at<const s32 *>(span, 0);
    u32 row = (u32)point[1] * at<u32>(surface, 0x24) + at<u32>(surface, 0x18) + (u32)point[0] * 2;
    const u16 packed = (u16)((colour >> 16 & 0xf8) << 8 | (colour >> 8 & 0xfc) << 3 | (colour & 0xff) >> 3);
    const s32 *rect = at<const s32 *>(span, 0xc);
    for (u8 rows = (u8)(rect[3] - rect[1]); rows; --rows) {
        const s32 *r = at<const s32 *>(span, 0xc);
        u16 *pixel = reinterpret_cast<u16 *>((uintptr_t)row);
        for (u8 cols = (u8)(r[2] - r[0]); cols; --cols) *pixel++ = packed;
        row += at<u32>(surface, 0x24);
    }
    *status = 0;
    return status;
}

s32 *__fastcall GlyphSolid_Span4444(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return Span<u16>(status, surface, span, Solid4444);
}

s32 *__fastcall GlyphSolid_Span8888(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return Span<u32>(status, surface, span, Solid8888);
}

s32 *__fastcall GlyphSolid_SpanBgra(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return Span<u32>(status, surface, span, [](u32 *p, u32 colour, u32 c) {
        // (the packing shifts the colour's byte 0 out: blue stays 0, sic)
        if (c) *p = c << 4 | c | (colour >> 8 & 0xff) << 16 | (colour >> 16 & 0xff) << 8;
    });
}

s32 *__fastcall GlyphPattern1_Span1555(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return PatternSpan<u16, true>(status, surface, span, Blend1555);
}

s32 *__fastcall GlyphPattern1_Span565(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return PatternSpan<u16, true>(status, surface, span, Blend565);
}

s32 *__fastcall GlyphPattern1_Span4444(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return PatternSpan<u16, true>(status, surface, span, [](u16 *p, u32 colour, u32 c) {
        // (unsigned divisions, where GlyphBlend's divide signed: a channel darker than the old one wraps, sic)
        const u32 old = *p;
        const u32 r = ((colour >> 12 & 0xf00) - (old & 0xf00)) * c / 15;
        const u32 g = ((colour >> 8 & 0xf0) - (old & 0xf0)) * c / 15;
        const u32 b = ((colour >> 4 & 0xf) - (old & 0xf)) * c / 15;
        *p = (u16)(((r + (old & 0xff00)) & 0xf00) | ((g + (old & 0xfff0)) & 0xf0) | ((b + old) & 0xf) | (old & 0xf000));
    });
}

s32 *__fastcall GlyphPattern1_Span8888(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return PatternSpan<u32, true>(status, surface, span, Blend8888);
}

s32 *__fastcall GlyphPattern1_SpanBgra(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return PatternSpan<u32, true>(status, surface, span, [](u32 *p, u32 colour, u32 c) {
        const u32 old = *p;
        const u32 g = ((colour & 0xff00) << 8) - (old & 0xff0000);
        const u32 r = ((colour >> 16 & 0xff) << 8) - (old & 0xff00);
        const u32 b = (colour << 24) - (old & 0xff000000);
        // (the old pixel's top byte ORed into the new one's, and the low byte cleared, sic)
        *p = ((g * c / 15 + (old & 0xffff0000)) & 0xff0000) | ((r * c / 15 + (old & 0xffffff00)) & 0xff00) |
             (((b * c / 15 + (old & 0xff000000)) | old) & 0xff000000);
    });
}

s32 *__fastcall GlyphPattern2_Span1555(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return PatternSpan<u16, true>(status, surface, span, [](u16 *p, u32 colour, u32 c) {
        // (the coverage's bit 0 tested and written at bit 12, in red, rather than as the alpha bit, sic)
        if (c & 1) *p = (u16)(0x1000 | (colour >> 9 & 0x7c00) | (colour >> 6 & 0x3e0) | (colour >> 3 & 0x1f));
    });
}

s32 *__fastcall GlyphPattern2_Span565(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return PatternSpan<u16, false>(status, surface, span, [](u16 *p, u32 colour, u32) {
        *p = (u16)((colour >> 9 & 0x7c00) | (colour >> 6 & 0x3e0) | (colour >> 3 & 0x1f));   // (packed as 555, sic)
    });
}

s32 *__fastcall GlyphPattern2_Span4444(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return PatternSpan<u16, true>(status, surface, span, Solid4444);
}

s32 *__fastcall GlyphPattern2_Span8888(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return PatternSpan<u32, true>(status, surface, span, Solid8888);
}

s32 *__fastcall GlyphPattern2_SpanBgra(void *, void *, s32 *status, const u8 *surface, const u8 *span) {
    return PatternSpan<u32, true>(status, surface, span, [](u32 *p, u32 colour, u32 c) {
        if (c) *p = colour << 24 | (colour & 0xff00) << 8 | (colour >> 16 & 0xff) << 8 | c << 4 | c;
    });
}
}
