#include "guifont.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"

namespace {

template <typename T> const T &at(const void *p, u32 offset) { return *reinterpret_cast<const T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall GuiFont_Init(u8 *font, void *, u32 count, u32 width, u32 spacing, u32 height, u32 b) {
    u8 *const glyphs = static_cast<u8 *>(GameHeap_Alloc((u32)((s32)(s16)count << 3)));
    *reinterpret_cast<u8 **>(font + 0xc) = glyphs;
    if (glyphs) {
        *reinterpret_cast<u16 *>(font + 4) = (u16)count;
        for (s16 i = 0; (s32)i < (s32)*reinterpret_cast<u16 *>(font + 4); ++i) {
            u8 *const glyph = *reinterpret_cast<u8 **>(font + 0xc) + i * 8;
            glyph[0] = (u8)width;
            glyph[1] = (u8)b;
            *reinterpret_cast<u32 *>(glyph + 4) = 0;
        }
        *reinterpret_cast<u16 *>(font + 0xa) = (u16)height;
    }
    *reinterpret_cast<u16 *>(font + 6) = (u16)spacing;
    return font;
}

u16 __fastcall GuiFont_TextWidth(const u8 *font, void *, const u16 *text, s16 from, s16 to) {
    u16 width = 0;
    const u8 *const glyphs = at<const u8 *>(font, 0xc);
    if (!text || !glyphs) return width;
    const s32 length = (s32)Crt().wcslen(text);
    if (to < from) to = (s16)(length - 1);
    if (length == 0 || from < 0 || from > length || to <= from || to > length) return width;
    for (s32 i = from; i <= to; ++i)
        width = (u16)(width + (s8)glyphs[text[i] * 8] + at<u16>(font, 6));
    return width;
}
}
