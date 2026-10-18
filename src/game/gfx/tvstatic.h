// The screen of static's surface: a 320 x 240 off-screen surface in system memory kept at 0x874fd0, filled with random black, white
// and grey pixels by 0x53e760 and stretched over the screen. Reimplementation of its creation 0x53e650 (with its caller 0x53e920) and
// its release 0x53e930. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "surface.h"

extern "C" {

// 0x53e650 (cdecl; the SEH frame left out): a Surface (0x50 bytes, the game's heap; when got, its colour key's five bytes +0x10 to +0x14,
// +0x20 and +0x24 cleared and its info constructed, SurfaceInfo_Ctor) and a DirectDraw surface for it: the IDirectDraw at [[0x879a24]]'s
// CreateSurface (COM slot 6) with a description of 0x6c bytes (flags 7: caps, height, width; 240 high, 320 wide, caps 0x840:
// off-screen plain in system memory; the rest of it is not set) and no outer unknown; its out slot holds the Surface until written.
// When that fails the result is -1, else Surface_Attach's (sic: with a null Surface when the allocation failed). A negative result
// frees the Surface (GameHeap_Free, even null) and returns 0; else it is returned.
Surface *__cdecl TvStatic_CreateSurface();

// 0x53e920 (cdecl): 0x874fd0 = TvStatic_CreateSurface().
void __cdecl TvStatic_Init();

// 0x53e930 (cdecl): the DirectDraw surface of the Surface at 0x874fd0 (Surface_GetSurface with addRef 1) released (COM slot 2), then the
// Surface freed (GameHeap_Free); 0x874fd0 keeps its value.
void __cdecl TvStatic_Free();
}
