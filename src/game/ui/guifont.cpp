#include "guifont.h"

#include <cstdint>

#include "../core/crt.h"

namespace {

template <typename T> const T &at(const void *p, u32 offset) { return *reinterpret_cast<const T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

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
