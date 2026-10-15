// The screen size and the search path, set up after the sine table (main loop, 0x45abbb):
// reimplementation of 0x449ec0 and 0x44a1d0.
//
// The search path table (0x79cb4c) holds one directory, a copy of the string at 0x79cb70
// (length 0x79cb6c) in a block of 0x79d73c bytes (0x79d25c); the file lookups (0x44a280) and the
// quickload loader read it. The screen is 0x7a1968 x 0x7a1930, its centre 0x7a0c9c / 0x7a162c
// (copied to 0x7a192c / 0x7a194c).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x449ec0: the search path table (GlobalMem_Alloc, zeroed; the string is copied whole, sic:
// unbounded), then the screen size: with 0x78d968 == 1 from the option flags 0x7a1a8c (the
// first of 0x20000000 without 1: 160x120, 0x40000000 320x240, 0x80000000 400x300, 8 512x384,
// 0x10 640x480, 0x20 800x600, 0x40 1024x768, 0x80 1280x1024, 0x100 1600x1200; else 640x480);
// else from Resolution.sam's RESOLUTION (0x7858ec; 1 512x384, 2 640x480, 3 800x600, 4 1024x768,
// 5 1280x1024, 6 1600x1200, 7 2048x1536, above 7 the size is left as it was, sic) or, when that
// is 0 or below, from 0x78d8dc (0 512x384, 2 800x600, else 640x480). Then the centre (half the
// size), 0x7a1a14 |= 8, Frame_ResetBounds and Quickload_Init.
void __cdecl Screen_Init();

// 0x44a1d0: rebuilds the sine table (0x4708d0, n = 4096: at the precision Direct3D left),
// Quickload_Free, then frees the path text and the table (each nulled; null skipped).
void __cdecl Screen_Shutdown();
}
