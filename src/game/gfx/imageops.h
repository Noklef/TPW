// Operations on images: cropping, dilating and eroding, the minimum filter, stamping one image onto another, tinting through a mask, blending an alpha image and the panel built from them. Reimplementation of 0x5d4170, 0x5d48d0, 0x5e7d70, 0x5e7f60, 0x5ea1f0, 0x5ea3f0, 0x5ea510, 0x5ea6d0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5d4170 (thiscall on the source image {width +0, height +4, ?, bytes +0xc}; dest, x, y, pops 12): the source's pixels of four bytes (alpha, then three
// channels) are blended onto the destination image ({width +0, height +4, bytes per pixel +8, bytes +0xc}) at column x and row y: the first byte of
// each destination pixel is 0xff and each channel d becomes d + ((s - d) * alpha >> 8) (arithmetic shift, low byte kept); the destination pointer
// moves on by 4 bytes per pixel and by (destination width - source width) * bytes per pixel at the end of each row.
void __fastcall Pixels_BlendOnto(const u8 *source, void *, u8 *dest, u32 x, u32 y);

// 0x5d48d0 (thiscall on the panel: images of {width, height, bytes per pixel, pixels at +0xc} 0x10 bytes apart): the image at +0x50 is filled with the pixel bytes 0xff, 0xfb,
// 0xf8, 0xc6 (width x height pixels of four bytes), then the images at +0x60, +0x20, +0x30, +0x40, +0x10 and +0 are blended onto it (Pixels_BlendOnto) at (0, 0), (0x220, 0),
// (0x1dd, 6), (0x141, 0xda), (4, 0x9f) and (0xb6, 0).
void __fastcall Panel_Reset(u8 *panel);

// 0x5d4260 (thiscall on the source image {width +0, height +4, ?, pixels of four bytes +0xc}; dest, width, height, format, pops 16; name unconfirmed): the
// source shrunk to width x height by box averages. The steps are (source size << 16 | 0x8000) / new size (unsigned); each new column and row covers the
// source columns / rows its step's top half moves on by (counts kept as bytes). Each row band is averaged across (each byte's sum, low 16 bits, over the
// count) into a scratch block of (row step + 1) x width pixels, reading the source straight on (by the counts, not by its width, sic), then down: bytes
// 1-3 of each column's average go to dest as format 1: bytes 3, 2, 1 and 0xff (4 bytes) or format 2: the u16 0xf000 | 1's top nibble << 8 | 2's << 4 |
// 3's (2 bytes); other formats write nothing. A count of 0 (a size grown) divides by 0, sic. Three scratch blocks (Mem_Alloc; any failing returns 0,
// those taken kept, sic), freed at the end; returns 1.
u8 __fastcall Image_ShrinkInto(const u8 *image, void *, u8 *dest, s32 width, s32 height, u32 format);

// 0x5ea3f0 (cdecl; destination image, source image, x, y; images {width, height, bytes per pixel, data}): the destination is made over (Array3D_Realloc: the source's width
// and height, one byte per pixel, cleared) and the part of the source that is left once it is moved by (x, y) (its rows and columns clamped to the source) is copied into it, the
// first byte of each pixel, at (max(x, 0), max(y, 0)). Returns true.
u8 __cdecl Image_CropInto(u8 *destination, u8 *source, s32 x, s32 y);

// 0x5ea510 / 0x5ea6d0 (cdecl; image {width, height, bytes per pixel, data}, radius): each row's first width bytes (packed, not stepping by the pixel size, sic) are replaced by the
// maximum / minimum of the window of the radius around each place (the window clamped to the row; the minimum writes 0 within the radius of the ends); then each column
// (bytes at the pixel size apart) the same. Uses a scratch block of the larger of the two sizes; returns 0 without one, else 1.
u32 __cdecl Image_Dilate(u32 *image, s32 radius);

u32 __cdecl Image_Erode(u32 *image, s32 radius);

// 0x5ea1f0 (cdecl; image, an unused second pointer, mark, mask image): the image's pixels are two bytes each (value, then room for a result): the result of each is the smallest of
// the value and its 3 x 3 neighbours that exist (the right ones only up to the column width - 3, below only for the rows up to height - 3, sic), and the mask image's byte of the
// pixel (in order, continuing across rows) is set to the mark where it is below the value; at the end each pixel's value becomes its result.
void __cdecl Image_MinMark(u32 *image, const u32 *unused, u8 mark, const u32 *maskImage);

// 0x5e7d70 (thiscall on the source image; destination image, x, y, pops 12): the source is drawn at (x, y), four destination bytes per pixel, by its pixel size: 1 byte v blends
// all four destination bytes towards v by v / 256; 2 bytes (a, b) store 0xff, a, b, 0; anything else reads four bytes (alpha, c1, c2, c3): destination byte 0 is the larger of it and
// alpha, bytes 1 and 2 blend towards c1 and c2 by alpha / 256 and byte 3 towards c3 (the fourth source byte). The destination rows advance by (destination width - source
// width) pixels of its own size.
void __fastcall Pixels_Stamp(u8 *source, void *, u8 *destination, s32 x, s32 y);

// 0x5e7f60 (thiscall on a one byte per pixel mask; destination image, scale, c1, c2, c3, pops 20; nothing when the mask has no data): for each mask pixel a = mask * scale >> 8:
// destination byte 0 becomes the larger of it and a, and bytes 1 to 3 blend by a / 256 towards c1, c2, c3 each multiplied by scale / 256.
void __fastcall Mask_Tint(u8 *mask, void *, u8 *destination, u32 scale, u32 c1, u32 c2, u32 c3);
}  // extern "C"
