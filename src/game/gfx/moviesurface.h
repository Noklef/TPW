// The movie player's DirectDraw helpers (the video library at 0x66e000-0x678000): the settings' (the player's dword +0) +0x10 the
// IDirectDraw, +0x14 / +0x18 / +0x1c / +0x20 its surfaces (+0x14 the primary), +4 / +8 the flags. COM calls through each object's
// vtable (GetCaps 0x2c, GetSurfaceDesc 0x58, IsLost 0x60, Restore 0x6c). Reimplementation of 0x670300, 0x670660, 0x671fa0, 0x670ad0, 0x671ea0, 0x671cb0 and 0x66fa70. The
// names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x670300 (thiscall on the player; name unconfirmed): the surface +0x14's description (0x6c bytes, cleared, its size set) read with
// GetSurfaceDesc; on success its width (+0xc) into the player's +4 and height (+8) into +8.
void __fastcall MovieSurface_ReadSize(u8 *player, void *edx);

// 0x670660 (thiscall on the player, returns the mode; name unconfirmed): 1 unless the flags +4 have bit 9 and the IDirectDraw's GetCaps
// (the driver's and the emulation's DDCAPS, 0x17c bytes each, cleared, their sizes set) succeeds; then 4 when the flags +8 have bit 2 and
// the driver's caps (+4) bits 8 and 9, else 2 when +8 has bit 1 and the caps bits 13 and 14, else 1.
u32 __fastcall MovieSurface_PickMode(u8 *player, void *edx);

// 0x671fa0 (thiscall on the player, returns 1 when a restore failed; name unconfirmed): each surface in turn (+0x14 when the flags +4
// have bit 25, +0x18 with bit 27, +0x1c and +0x20 always; skipped when null): when IsLost answers DDERR_SURFACELOST (0x887601c2) it is
// read again and restored; a failed Restore answers 1 at once, and a restored +0x1c / +0x20 sets the player's +0x50 to 1 when its +0x4c
// is 2. Else 0. The settings are read again for each step.
u32 __fastcall MovieSurface_Restore(u8 *player, void *edx);

// The surfaces made for the movie (fastcall (draw, out, ...), the caller's IDirectDraw; return 0, or 1 when a step failed; names unconfirmed). Each: a description (0x6c bytes, cleared; flags 0x1007, its width, height and pixel
// format size 0x20, flags 4, the FourCC) with CreateSurface (0x18), its later-interface pointer (QueryInterface for the GUID 0x707220) and the first released, then filled grey (Blt, 0x14: colour fill 0x80808080; the result ignored).
// 0x670ad0: caps 0x80 (an overlay), the colour key {1, 1} (SetColorKey, 0x74, flags 4). 0x671ea0: caps 0x4040 (off-screen in video memory), no key. A QueryInterface that fails releases the first.
u32 __fastcall MovieSurface_CreateOverlay(u8 *draw, u8 **out, u32 width, u32 height, u32 fourcc);
u32 __fastcall MovieSurface_CreateOffscreen(u8 *draw, u8 **out, u32 width, u32 height, u32 fourcc);
// 0x671cb0: flags 0x1027, caps 0x98 (a flipping overlay), one back buffer got with GetAttachedSurface (0x30, caps 4); both moved to the later interface, keyed and filled; on a failure those of the first two still held are released
// (the later interfaces are not, sic), and the outs are written only on success.
u32 __fastcall MovieSurface_CreateFlipOverlay(u8 *draw, u8 **out, u8 **backOut, u32 width, u32 height, u32 fourcc);

// 0x66fa70 (fastcall (player, show); returns 0 or 1; name unconfirmed): the settings' surface +0x1c shown over +0x14 with UpdateOverlay (0x84): the source rectangle from 0 sized settings +0x64, +0x60, the destination from (player +0x14,
// +0x18) sized +0xc, +0x10, the effects (0x38 bytes, cleared; show: +0x30 1, flags 0x84000; else flags 0x200). A failure sets the player's +0x4c to 1 for five errors (invalid or unsupported ones), restores the surface +0x1c (Restore,
// 0x6c) on DDERR_SURFACELOST, and answers 1.
u32 __fastcall MovieSurface_UpdateOverlay(u8 *player, u32 show);
}
