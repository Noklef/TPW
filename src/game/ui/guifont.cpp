#include "guifont.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/savegame.h"
#include "../core/vcall.h"
#include "../core/initializers.h"

namespace {

template <typename T> const T &at(const void *p, u32 offset) { return *reinterpret_cast<const T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall GuiFont_Dtor(u8 *font) {
    *reinterpret_cast<u32 *>(font) = (u32)(uintptr_t)Class7058d8_vtable;
    u8 *const glyphs = *reinterpret_cast<u8 **>(font + 0xc);
    if (glyphs) GameHeap_Free(glyphs);
}

void __fastcall GuiFont_FreeGlyphs(u8 *font) {
    u8 *&glyphs = *reinterpret_cast<u8 **>(font + 0xc);
    if (!glyphs) return;
    GameHeap_Free(glyphs);
    glyphs = nullptr;
}

u8 *__fastcall GuiFont_DeletingDtor(u8 *font, void *, u32 flags) {
    GuiFont_Dtor(font);
    if (flags & 1) GameHeap_Free(font);
    return font;
}

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

void __fastcall GuiFont_SetGlyph(u8 *font, void *, u32 index, u32 width, u32 b) {
    u8 **const glyphs = reinterpret_cast<u8 **>(font + 0xc);   // (read again for each store, as the original)
    if (*glyphs && (s16)index < (s32)*reinterpret_cast<u16 *>(font + 4)) {   // (a negative index writes before the glyphs, sic)
        (*glyphs)[(s16)index * 8] = (u8)width;
        (*glyphs)[(s16)index * 8 + 1] = (u8)b;
    }
}

u16 __fastcall GuiFont_CharWidth(const u8 *font, void *, u32 character) {
    const u8 *const glyphs = at<const u8 *>(font, 0xc);
    const u32 c = character & 0xffff;
    if (!glyphs || c >= at<u16>(font, 4)) return 0;
    return (u16)((s8)glyphs[c * 8] + at<u16>(font, 6));
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

void __fastcall GuiFont_SetSpacing(u8 *font, void *, u32 across, u32 down) {
    // (the original tests each word, zero-extended, for below 0 first: it never is)
    *reinterpret_cast<u16 *>(font + 6) = (u16)across;
    *reinterpret_cast<u16 *>(font + 8) = (u16)down;
}

}  // extern "C"

namespace {

// The glyph's two bytes (+0xc read each time, as the original does).
const s8 *glyph(const u8 *font, s32 index) { return reinterpret_cast<const s8 *>(at<const u8 *>(font, 0xc) + index * 8); }

// The folded stub 0x416e10 the original calls for each character it draws (Save_FieldName: 0); the new pen, x in the high word.
void draw(s16 &cx, s16 &cy, u16 ch) {
    const u32 r = Save_FieldName((u16)cx, reinterpret_cast<const char *>((uintptr_t)(u16)cy), ch);
    cx = (s16)((s32)r >> 16);
    cy = (s16)r;
}

u32 pen(s16 cx, s16 cy) { return (u32)(s32)cx << 16 | (u32)(s32)cy; }

// The clip's slot 0 (thiscall (x, y)): AX nonzero.
bool inside(u8 *clip, u32 x, u32 y) { return vcall<u16>(clip, 0, x, y) != 0; }

}  // namespace

extern "C" {

u32 __fastcall GuiFont_DrawText(const u8 *font, void *, u32 x, u32 y, const u16 *text, u32) {
    s16 cx = (s16)x, cy = (s16)y;
    if (at<u32>(font, 0xc))
        for (s16 i = 0; text[i]; ++i)
            if (text[i] < at<u16>(font, 4)) draw(cx, cy, text[i]);
    return pen(cx, cy);
}

u32 __fastcall GuiFont_DrawTextClipped(const u8 *font, void *, u32 x, u32 y, const u16 *text, u8 *clip, u32 clipped) {
    s16 cx = (s16)x, cy = (s16)y, i = 0;
    if (clipped) {
        if (!text) return pen(cx, cy);
        // (the original pushes y's low word in EAX, whose upper half is what EAX held on entry: 0 here, as in the benches; the clip
        // tests words)
        if (!inside(clip, (s32)at<s16>(clip, 4) + 1, (u16)y)) return pen(cx, cy);
        if (!inside(clip, (s32)at<s16>(clip, 4) + 1, (s32)cy + at<u16>(font, 0xa))) return pen(cx, cy);
        s32 first = -1, last = -1;
        s16 saved = 0;  // (unset when no character shows: the original's stack slot, sic)
        const s32 length = (s32)Crt().wcslen(text);
        for (; first == -1 && i < length; ++i) {
            const s16 ch = (s16)text[i];
            if (ch >= (s32)at<u16>(font, 4)) continue;
            // (the first test's words carry the upper halves the original leaves in EDX, the font's, and ECX, the character's sign)
            if (inside(clip, ((u32)(uintptr_t)font & 0xffff0000) | (u16)cx, (ch < 0 ? 0xffff0000 : 0) | (u16)cy) &&
                inside(clip, cx + glyph(font, ch)[0] - 1, cy + glyph(font, ch)[1] - 1)) {
                if (first == -1) first = i, saved = cx;
                last = i;
            }
            cx = (s16)(cx + glyph(font, ch)[0] + at<u16>(font, 6));
        }
        cx = saved;
        if (first < 0 || last <= first) return pen(cx, cy);  // (the loop stops at the first character shown, so last is first: sic)
        for (i = (s16)first; i <= last; ++i)
            if (text[i] < at<u16>(font, 4)) draw(cx, cy, text[i]);
        return pen(cx, cy);
    }
    for (;; ++i) {
        const u16 ch = text[i];
        if (!ch) break;
        if (ch >= at<u16>(font, 4)) continue;
        if (inside(clip, (s32)cx + 1, (s32)cy + 1) && inside(clip, cx + glyph(font, ch)[0] - 1, cy + glyph(font, ch)[1] - 1)) {
            draw(cx, cy, ch);
            continue;
        }
        cx = (s16)(cx + glyph(font, ch)[0] + at<u16>(font, 6));
    }
    return pen(cx, cy);
}
}
