// A sign's text drawn by GDI and Uniscribe into the sign renderer's images.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5eab70 (thiscall; device context, wide text, image {width, height, bytes per pixel, pixels}, pops 12; AL; name invented, unconfirmed).
// self: +0 first row with text, +4 the row after the last, +8 the spacing in percent, +0xc the text's y, +0x10 its LOGFONT.
// No text: the rows 0, the pixels cleared, 1. Otherwise an 8-bit grey section of twice the image's size (no section: 0). The text is shaped with
// the widths at 0xf85f00 scaled by the spacing; when it is 2 * width wide or more, a search on the font's width (+0x14, or -height when it is 0)
// for one that fits. Drawn white centred on x 0x100, the image remade at the same size, 1 byte a pixel, each pixel the mean of 4 section bytes.
// The rows of text found again from the pixels, a pixel only in the last column not counting (sic). Returns 1.
u8 __fastcall SignRender_Text(u8 *self, void *, u32 dc, const u16 *text, u8 *image);
}
