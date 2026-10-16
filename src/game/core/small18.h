// The seven-array object's constructor and an image blend: reimplementation of 0x5d45b0 and 0x5d4170. Named by what they do until the classes are
// known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5d45b0 (thiscall): all 0x70 bytes of the object (seven groups of 16 from +0; see Blocks7_Free in small16) cleared. Returns the object.
u8 *__fastcall Blocks7_Ctor(u8 *object);

// 0x5d4170 (thiscall on the source image {width +0, height +4, ?, bytes +0xc}; dest, x, y, pops 12): the source's pixels of four bytes (alpha, then three
// channels) are blended onto the destination image ({width +0, height +4, bytes per pixel +8, bytes +0xc}) at column x and row y: the first byte of
// each destination pixel is 0xff and each channel d becomes d + ((s - d) * alpha >> 8) (arithmetic shift, low byte kept); the destination pointer
// moves on by 4 bytes per pixel and by (destination width - source width) * bytes per pixel at the end of each row.
void __fastcall Pixels_BlendOnto(const u8 *source, void *, u8 *dest, u32 x, u32 y);

}  // extern "C"
