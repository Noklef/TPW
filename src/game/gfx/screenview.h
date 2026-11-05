// The screen's surface as a drawing target (the methods of vtable 0x703720, 0x6317e0-0x631860), and the pixel formats' global tables filled at start (0x6316c0-0x6317a0).
// Names invented.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

extern "C" {

// 0x6316c0 / 0x631700 / 0x631750 / 0x6317a0 (the dynamic initializers' bodies): the tables at 0xfaa298 (8-bit paletted), 0xfaa2b0 (555), 0xfaa268 (888), 0xfaa280 (8888).
void __cdecl PixelFormats_InitPal8();
void __cdecl PixelFormats_Init555();
void __cdecl PixelFormats_Init888();
void __cdecl PixelFormats_Init8888();

// 0x6317e0 (thiscall, no object used): the screen's surface 0xfa7780.
u32 __cdecl ScreenView_Surface();
// 0x6317f0: the dword at 0xfa78e4.
u32 __cdecl ScreenView_Field164();
// 0x631800 (thiscall; rect out, pops 4): {0, 0, width, height} of the surface the object's slot 8 gives (Surface_Refresh; its +0x1c, +0x20); all 0 without a DirectDraw object. Returns the out.
u32 *__fastcall ScreenView_Rect(u8 *self, void *, u32 *out);
// 0x631860: bit 1 of the dword at 0xfa779c.
u32 __cdecl ScreenView_Flag1();

}  // extern "C"

void ScreenView_SetCalls(const AddrCall *table, u32 count);
