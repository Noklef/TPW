// The surface format helpers (0x61b090-0x61b1d2): a DirectDraw surface's description kept as a
// SurfaceInfo, and the shifts that move each colour channel of a pixel format to or from
// 8 bits per channel (0xAARRGGBB).
//
// The originals' quirks are kept: a channel with an empty mask counts as topping out at bit 31;
// an 8-bit (palettized) description leaves the masks as they were; Refresh keeps the old bits
// pointer when the surface describes itself without one, and returns the info unchanged when the
// description fails.
#pragma once

#include <cstddef>

#include "../../../re/types.h"

// One channel: its mask, and how far its top bit sits above (down) or below (up) the top bit it
// has in 0xAARRGGBB.
struct PixelChannel {
    u32 mask;
    u16 down;  // +4
    u16 up;    // +6
};
struct PixelShifts {  // the global one is at 0xfa7708
    PixelChannel red, green, blue, alpha;
};
static_assert(sizeof(PixelShifts) == 0x20, "PixelShifts is 0x20 bytes");

struct SurfaceInfo {
    u32 redMask, greenMask, blueMask, alphaMask;
    u32 bitCount;   // +0x10
    u32 paletted;   // +0x14: 1 for 8 bits
    void *bits;     // +0x18
    u32 width;      // +0x1c
    u32 height;     // +0x20
    s32 pitch;      // +0x24
};
static_assert(sizeof(SurfaceInfo) == 0x28, "SurfaceInfo is 0x28 bytes");

// The part of the surface wrapper these use (its first 0x24 bytes are unknown).
struct Surface {
    u8 unknown[0x24];
    void *surface;     // +0x24: the IDirectDrawSurface
    SurfaceInfo info;  // +0x28
};
static_assert(offsetof(Surface, info) == 0x28, "Surface::info is at +0x28");

extern "C" {
// 0x61b090 (mask, top): the mask's top bit against top (31, 23, 15 or 7).
void __fastcall PixelChannel_Set(PixelChannel *c, void *edx, u32 mask, u32 top);
// 0x61b0e0 (&masks: red, green, blue, alpha): alpha, red, green, blue in that order.
void __fastcall PixelShifts_Set(PixelShifts *s, void *edx, const u32 *masks);
// 0x61b130 (&DDSURFACEDESC): bits, width, height, pitch, bit count, then the masks unless 8 bits.
void __fastcall Surface_SetInfo(Surface *s, void *edx, const u8 *desc);
// 0x61b190: the surface's GetSurfaceDesc (+0x58) into the info; the info.
SurfaceInfo *__fastcall Surface_Refresh(Surface *s);
}
