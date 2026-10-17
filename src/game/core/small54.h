// Two blends onto a four byte per pixel image: reimplementation of 0x5e7d70 and 0x5e7f60. Images are {width, height, bytes per pixel, data}. Named by what they do until the
// callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5e7d70 (thiscall on the source image; destination image, x, y, pops 12): the source is drawn at (x, y), four destination bytes per pixel, by its pixel size: 1 byte v blends
// all four destination bytes towards v by v / 256; 2 bytes (a, b) store 0xff, a, b, 0; anything else reads four bytes (alpha, c1, c2, c3): destination byte 0 is the larger of it and
// alpha, bytes 1 and 2 blend towards c1 and c2 by alpha / 256 and byte 3 towards c3 (the fourth source byte). The destination rows advance by (destination width - source
// width) pixels of its own size.
void __fastcall Pixels_Stamp(u8 *source, void *, u8 *destination, s32 x, s32 y);

// 0x5e7f60 (thiscall on a one byte per pixel mask; destination image, scale, c1, c2, c3, pops 20; nothing when the mask has no data): for each mask pixel a = mask * scale >> 8:
// destination byte 0 becomes the larger of it and a, and bytes 1 to 3 blend by a / 256 towards c1, c2, c3 each multiplied by scale / 256.
void __fastcall Mask_Tint(u8 *mask, void *, u8 *destination, u32 scale, u32 c1, u32 c2, u32 c3);

}  // extern "C"
