// The sound occluders (the list at 0xfb2038; the names are invented): a pointer list (+4 its head, nodes of {occluder, next}; +0xc its
// count; ptrlist.h's PtrList_InsertCopyByKey keeps it sorted by the radius) of occluders, circles in the xz plane: floats +0 x, +4 z,
// +0xc the radius; +0x14 the smallest radius that counts. Reimplementation of 0x6c4c80.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c4c80 (thiscall (from, to, out), pops 0xc): the occlusion of the path from `from` to `to` (floats x at +0, z at +8), the largest of the
// occluders' until one smaller than +0x14 (or NaN; that ends the walk). An occluder counts only when `to` is not inside it (squared
// distance, from the absolute differences, at or above the squared radius, or NaN); then the point of the line through the two nearest it
// (the parameter stored as a float; x clamped to the segment's x span, z with Float_ClampBetween) gives 1 - d^2 / r^2 when d^2 is not above
// r^2 (NaN counts), stored through out, kept when above the largest so far. *out = the largest (0 to start with); returns 1 when it is
// neither 0 nor NaN, else 0. All in the x87's order of operations and stores.
u32 __fastcall SoundOccluders_Occlusion(u8 *list, void *, const float *from, const float *to, float *out);

}  // extern "C"
