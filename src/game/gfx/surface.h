// The surface format helpers (0x61b090-0x61b1d2): a DirectDraw surface's description kept as a
// SurfaceInfo, and the shifts that move each colour channel of a pixel format to or from
// 8 bits per channel (0xAARRGGBB). Also SurfaceInfo's own methods (0x5fa680-0x5fa897) and the
// surface wrapper's DirectDraw calls (0x5f8f80-0x5f963e).
//
// The originals' quirks are kept: a channel with an empty mask counts as topping out at bit 31;
// an 8-bit (palettized) description leaves the masks as they were; Refresh keeps the old bits
// pointer when the surface describes itself without one, and returns the info unchanged when the
// description fails.
#pragma once

#include <cstddef>

#include "../../../re/types.h"
#include "../math/rect.h"

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

// The surface wrapper: an IDirectDrawSurface, its description and a colour key.
struct Surface {
    u32 unknown00[3];  // zeroed by InitHeader
    u32 unknown0c;
    u32 keyRgb;        // +0x10: the colour key, 0xAARRGGBB (for more than 8 bits)
    u8 keyIndex;       // +0x14: the colour key's palette index (8 bits)
    u8 pad15[3];
    const void *palette;  // +0x18: the game's palette (0xfa71f8) for 8 bits, else 0
    u32 unknown1c;     // InitHeader's last argument (0 from every caller here)
    u32 attached;      // +0x20: 1 when it wraps a surface made elsewhere (Attach)
    void *surface;     // +0x24: the IDirectDrawSurface
    SurfaceInfo info;  // +0x28
};
static_assert(offsetof(Surface, info) == 0x28, "Surface::info is at +0x28");
static_assert(sizeof(Surface) == 0x50, "Surface is 0x50 bytes");

extern "C" {
// 0x61b090 (mask, top): the mask's top bit against top (31, 23, 15 or 7).
void __fastcall PixelChannel_Set(PixelChannel *c, void *edx, u32 mask, u32 top);
// 0x61b0e0 (&masks: red, green, blue, alpha): alpha, red, green, blue in that order.
void __fastcall PixelShifts_Set(PixelShifts *s, void *edx, const u32 *masks);
// 0x61b130 (&DDSURFACEDESC): bits, width, height, pitch, bit count, then the masks unless 8 bits.
void __fastcall Surface_SetInfo(Surface *s, void *edx, const u8 *desc);
// 0x61b190: the surface's GetSurfaceDesc (+0x58) into the info; the info.
SurfaceInfo *__fastcall Surface_Refresh(Surface *s);

// SurfaceInfo as a bitmap of its own (0x5fa680-0x5fa897, thiscall). The ones with a result
// write 0 or -1 through out and return out. Bytes a pixel are (bitCount + 7) >> 3.
// 0x5fa680: zeroes everything from bitCount on (the masks are left); returns i.
SurfaceInfo *__fastcall SurfaceInfo_Ctor(SurfaceInfo *i);
// 0x5fa6a0 (&{width, height}, &format: the masks, bitCount and paletted, bits, pitch): pitch 0
// means width pixels; returns i.
SurfaceInfo *__fastcall SurfaceInfo_Init(SurfaceInfo *i, void *edx, const u32 *size, const u32 *format, void *bits,
                                         s32 pitch);
// 0x5fa6f0 (&out): GlobalAlloc'd bits of width pixels a row; -1 when it already has bits.
s32 *__fastcall SurfaceInfo_Alloc(SurfaceInfo *i, void *edx, s32 *out);
void __fastcall SurfaceInfo_Free(SurfaceInfo *i);  // 0x5fa740: frees the bits (if any)
// 0x5fa760 (&out, dst, &{x, y}, &{width, height}): SubRect of that rectangle.
s32 *__fastcall SurfaceInfo_SubRectAt(const SurfaceInfo *i, void *edx, s32 *out, SurfaceInfo *dst, const s32 *at,
                                      const s32 *size);
// 0x5fa7b0 (&out, dst, &rect): dst views the part of i inside rect (an empty part collapsed to
// its top left); -1 when that is empty.
s32 *__fastcall SurfaceInfo_SubRect(const SurfaceInfo *i, void *edx, s32 *out, SurfaceInfo *dst, const Rect *r);
u32 __fastcall SurfaceInfo_Size(const SurfaceInfo *i);  // 0x5fa890: height * pitch

// The surface wrapper's DirectDraw calls (0x5f8f80-0x5f963e, cdecl). The ones with a result write
// it through out (0, -1 or a failed Lock's HRESULT) and return out. A call that finds the surface
// lost (DDERR_SURFACELOST) restores it; only Lock tries again.
void __cdecl Surface_SetDirectDraw(void *directDraw);  // 0x5f8f80: the IDirectDraw Create uses
// 0x5f8f90 (&out, IDirectDrawSurface, s): s zeroed, then wraps the surface (attached).
s32 *__cdecl Surface_Attach(s32 *out, void *surface, Surface *s);
// 0x5f9000 (s, width, height, bit count, x): the header (the size is not kept); returns s.
Surface *__cdecl Surface_InitHeader(Surface *s, u32 width, u32 height, u32 bitCount, u32 unknown1c);
// 0x5f9040: 0 when the surface was lost and could not be restored, else 1.
u32 __cdecl Surface_Restore(Surface *s);
// 0x5f9070 (&out, width, height, caps, s): an offscreen plain surface (caps | 0x40).
s32 *__cdecl Surface_Create(s32 *out, u32 width, u32 height, u32 caps, Surface *s);
s32 *__cdecl Surface_Release(s32 *out, Surface *s);  // 0x5f9130: 0
// 0x5f9160 (&out, dst, x, y, src, &source rect or 0 for all of src, DDBLTFAST flags): flag 1 sets
// src's colour key first; DDBLTFAST_WAIT (0x10) is toggled.
s32 *__cdecl Surface_BltFast(s32 *out, Surface *dst, u32 x, u32 y, Surface *src, const Rect *srcRect, u32 flags);
// 0x5f92e0 (&out, dst, &dst rect, src, &src rect, DDBLT flags): a 0 rect means all of the surface;
// 0x8000 sets src's colour key first; DDBLT_WAIT (0x1000000) is toggled; 0x800 asks for
// DDBLTFX_NOTEARING.
s32 *__cdecl Surface_Blt(s32 *out, Surface *dst, const Rect *dstRect, Surface *src, const Rect *srcRect, u32 flags);
// 0x5f9550 (&out, s, &bits or 0): locks all of it (DDLOCK_WAIT, | DDLOCK_NOSYSLOCK when 0xfa76b4
// is set) and takes the description; out gets a failed Lock's HRESULT.
s32 *__cdecl Surface_Lock(s32 *out, Surface *s, void **bits);
s32 __cdecl Surface_Unlock(Surface *s);                // 0x5f9600: Unlock(0)'s result
void *__cdecl Surface_GetSurface(Surface *s, s32 addRef);  // 0x5f9620: the IDirectDrawSurface
}

// What the wrapper uses in the game.
struct SurfaceEnv {
    void **directDraw;     // 0xfa76b0
    const u8 *noSysLock;   // 0xfa76b4
    const void *palette;   // 0xfa71f8
};
// Surface_* go through env (nullptr: the game's).
void Surface_SetEnv(const SurfaceEnv *env);
