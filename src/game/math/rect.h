// The rectangle helpers (0x61af20-0x61b029, cdecl): a Win32-style RECT {left, top, right,
// bottom}, the right and bottom edges exclusive. Comparisons are signed.
//
// The originals' quirks are kept: Intersect writes the intersection into its first rectangle
// even when it is empty (and answers 0 then).
#pragma once

#include "../../../re/types.h"

struct Rect {
    s32 left, top, right, bottom;
};
static_assert(sizeof(Rect) == 0x10, "Rect is 0x10 bytes");

extern "C" {
void __cdecl Rect_Copy(Rect *dst, const Rect *src);  // 0x61af20
s32 __cdecl Rect_Width(const Rect *r);               // 0x61af40: right - left
s32 __cdecl Rect_Height(const Rect *r);              // 0x61af50: bottom - top
// 0x61af60: a becomes a and b's intersection; 1 when it is not empty.
u32 __cdecl Rect_Intersect(Rect *a, const Rect *b);
void __cdecl Rect_Collapse(Rect *r);   // 0x61afe0: right = left, bottom = top
void __cdecl Rect_Normalize(Rect *r);  // 0x61aff0: left <= right and top <= bottom, by swapping
// The thiscall methods (0x62f810-0x62f975). Clip and Union read both rectangles before writing
// out, so out may be either of them; an empty Clip collapses to width or height 0 at its top left.
Rect *__fastcall Rect_Clip(const Rect *a, void *edx, Rect *out, const Rect *b);   // 0x62f810: out = a & b
u32 __fastcall Rect_Overlaps(const Rect *a, void *edx, const Rect *b);           // 0x62f890: edges inclusive
Rect *__fastcall Rect_Union(const Rect *a, void *edx, Rect *out, const Rect *b);  // 0x62f8d0: bounding box
void __fastcall Rect_Order(Rect *r);  // 0x62f940: Rect_Normalize as a method (by xor swaps)
}
