// A panel of images: reimplementation of 0x5d48d0. Named by what it does until the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5d48d0 (thiscall on the panel: images of {width, height, bytes per pixel, pixels at +0xc} 0x10 bytes apart): the image at +0x50 is filled with the pixel bytes 0xff, 0xfb,
// 0xf8, 0xc6 (width x height pixels of four bytes), then the images at +0x60, +0x20, +0x30, +0x40, +0x10 and +0 are blended onto it (Pixels_BlendOnto) at (0, 0), (0x220, 0),
// (0x1dd, 6), (0x141, 0xda), (4, 0x9f) and (0xb6, 0).
void __fastcall Panel_Reset(u8 *panel);

}  // extern "C"
