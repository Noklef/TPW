// The separable grow and shrink filters of an image: reimplementation of 0x5ea510 and 0x5ea6d0. Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5ea510 / 0x5ea6d0 (cdecl; image {width, height, bytes per pixel, data}, radius): each row's first width bytes (packed, not stepping by the pixel size, sic) are replaced by the
// maximum / minimum of the window of the radius around each place (the window clamped to the row; the minimum writes 0 within the radius of the ends); then each column
// (bytes at the pixel size apart) the same. Uses a scratch block of the larger of the two sizes; returns 0 without one, else 1.
u32 __cdecl Image_Dilate(u32 *image, s32 radius);
u32 __cdecl Image_Erode(u32 *image, s32 radius);

}  // extern "C"
