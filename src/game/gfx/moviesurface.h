// The movie player's DirectDraw helpers (the video library at 0x66e000-0x678000): the settings' (the player's dword +0) +0x10 the
// IDirectDraw, +0x14 / +0x18 / +0x1c / +0x20 its surfaces (+0x14 the primary), +4 / +8 the flags. COM calls through each object's
// vtable (GetCaps 0x2c, GetSurfaceDesc 0x58, IsLost 0x60, Restore 0x6c). Reimplementation of 0x670300, 0x670660 and 0x671fa0. The
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
}
