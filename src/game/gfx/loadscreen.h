// The loading screen: reimplementation of 0x5879c0 and 0x586d70.
//
// The loading screen is the object at 0x8c4748 (its static initializer 0x586d40: +0 = 0, +4 = -1, +8 = 500; 0x586d70 its atexit
// destructor). 0x587380 shows it from a folder ("data\init", 0x54dea9): it counts its shows at 0x8c478c (0x587963), sets bit 0x40000000 of
// 0x8bcbc8 and takes timeGetTime at 0x8c4780 when shown first (0x587d0d). +0x10 and +0x14 point to tracked blocks (a dword handle each,
// as TrackedBlock_Dtor's, core/gameheap).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x587130 (thiscall on an image: +0 its pixels, +4 width, +8 height, +0xc bytes a pixel; name unconfirmed; 0x587380 and 0x587d00 show
// the loading screen's image with it): nothing when the pixels are null. Display driver kind (0x8bd524) 1: the display's (0x879a24)
// +0x14 surface is locked (COM slot 0x64: a zeroed 0x7c-byte description, size 0x7c, flags 1; failing returns at once), each of its
// rows (height +8, width +0xc, pitch +0x10, bits +0x24, s32 counts) filled from the image stretched and upside down: pixel
// [((H - y - 1) * height / H) * width + x * width / W] (unsigned divisions), u16 when 2 bytes a pixel else u32; then unlocked (slot
// 0x80). Kind 4: the same into the renderer's buffer ([0x879dac] + 8, u32, rows packed) of 0x8bcbd4 rows and 0x8bcbd0 columns,
// signed divisions. Returns 1.
u32 __fastcall LoadScreen_ShowImage(u8 *image);

// 0x5879c0 (thiscall; name unconfirmed): once shown ([0x8c478c] not 0), timeGetTime (the import at 0x6fd3ec) is asked until 3000 ms
// have passed since [0x8c4780] (unsigned). Then the blocks at +0x10 and +0x14, each when set: its handle freed (TrackedGlobal_Free) and
// cleared when set, the block freed (GameHeap_Free) and the field cleared. Bit 0x40000000 of 0x8bcbc8 is cleared. Returns 1.
u32 __fastcall LoadScreen_Close(u8 *screen);

// 0x586d70 (cdecl; the atexit destructor of the loading screen at 0x8c4748; name unconfirmed): LoadScreen_Close's steps on it, but
// returning nothing.
void __cdecl LoadScreen_StaticDtor();

// 0x587a70 (thiscall on the loading screen; name unconfirmed; 0x587d4d after counting a step at +0): the progress bar drawn. The percentage is
// the step count (+0, s32) times +0x18 (x87, at the game's precision) truncated; the bar 318 wide past 100, else percentage * 318 / 100. With the
// screen's width W (0x8bcbd0) and height H (0x8bcbd4), each scaled with signed 32-bit wrapping divisions: scale (W << 16) / 640, b 11H / 480,
// left 14W / 640, top 446H / 480, e b / 5, f 4b / 5, right ((bar + 1) * scale >> 16) + left. The renderer ([0x879ab0]) is begun (slot 0x1c),
// then three rectangles (slot 0x28: x0, y0, x1, y1 as floats, colour, 0): (left, top, right, top + e) 0xff800000, (left, e + top, right,
// f + top) 0xffff0000, (left, f + top, right, top + b) 0xff800000, where e + top and f + top are the integer added to top as a float on the x87
// in the second's and third's y0 and integers added elsewhere. A rectangle refused (0) returns 0 at once; else the renderer is ended (slot
// 0x20), +4 and +0xc = bar and 1 returned.
u32 __fastcall LoadScreen_DrawBar(u8 *screen);
}
