// A sign's text drawn by GDI and Uniscribe into the sign renderer's images.
#pragma once

#include "../../../re/types.h"

// The callees of SignRender_Draw (tests/fncmp/signrenderdraw.py gives them recorders).
struct SignRenderEnv {
    u8(__fastcall *text)(u8 *self, void *, u32 dc, const u16 *text, u8 *image);  // SignRender_Text
    u8(__fastcall *filter)(u8 *self, void *, u32 *source, u8 *dest);             // Bump_FilterInto
    void(__fastcall *build)(u8 *self);                                           // Bump_Build
};
void SignRender_SetEnv(const SignRenderEnv *env);  // nullptr: the game's

extern "C" {

// 0x5eab70 (thiscall; device context, wide text, image {width, height, bytes per pixel, pixels}, pops 12; AL; name invented, unconfirmed).
// self: +0 first row with text, +4 the row after the last, +8 the spacing in percent, +0xc the text's y, +0x10 its LOGFONT.
// No text: the rows 0, the pixels cleared, 1. Otherwise an 8-bit grey section of twice the image's size (no section: 0). The text is shaped with
// the widths at 0xf85f00 scaled by the spacing; when it is 2 * width wide or more, a search on the font's width (+0x14, or -height when it is 0)
// for one that fits. Drawn white centred on x 0x100, the image remade at the same size, 1 byte a pixel, each pixel the mean of 4 section bytes.
// The rows of text found again from the pixels, a pixel only in the last column not counting (sic). Returns 1.
u8 __fastcall SignRender_Text(u8 *self, void *, u32 dc, const u16 *text, u8 *image);

// 0x5ecb40 (thiscall; device context, sign {first text, second text, pixels 1, pixels 2, width, height, format}, pops 8; AL; name invented, unconfirmed).
// Any of the first four null: 0. The images +0x28 (4 bytes), +0x384 and +0x3f0 (1 byte) remade at twice the size, the texts drawn into the last two, each filtered
// by its mode (+0xc, +0x10) into a local mask or the mask cleared, the bump steps +0x358 and +0x3c4 run. On +0x28 cleared: the picture +0x18 when the byte +8 is set; mode 2's
// masks tinted (with both 2 their larger bytes, by the first colour); the lit pictures and mode 1's masks in the order the dword +4 chooses. Format 1: h rows of +0x28, w pixels
// each to pixels 1 and 2, bottom up, bytes reversed; format 2 the same at 4 bits a channel. Returns 1.
u8 __fastcall SignRender_Draw(u8 *self, void *, u32 dc, u8 *sign);
}
