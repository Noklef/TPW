// The display's DirectDraw start-up (0x563460-0x564097; name invented, unconfirmed). The object: +0 IDirectDraw, +4 IDirectDraw2, +8 IDirectDraw4, +0xc the clipper, +0x10 the primary surface, +0x14 the back or
// off-screen surface, +0x20 the device identifier (driver, description, version +0x400, vendor and device +0x408), +0x448 up to four drivers' GUIDs, +0x488 and +0x604 the two capability structures (0x17c bytes each),
// +0x780 bit 0: full screen, +0x788 the drivers found, +0x78c the one in use.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x563460 (thiscall; width, height, depth, driver, full screen, video memory; pops 0x18): the render view's screen set and DirectDraw up; 1, or 0 with the error pushed and the object's COM members released.
//   Scene flag 0x800000 of [0x8bd534] clear: the error (3, 0xb0754017) and 0.
//   No IDirectDraw yet: made on the driver asked for (the drivers listed and sorted by GUID, strncmp; scene flag 0x10 of [0x8bd508]: the default driver, its only one), queried for IDirectDraw2 and 4
//   (4 missing: the error (3, 0xb0754016) as well), its identifier and capabilities read. One there on another driver: the members released, made and queried for 4 again.
//   Full screen 1: exclusive, the mode set, the window shown (ShowWindow 3), a primary with one back buffer (two by bit 0 of the view) in video memory or not, cleared (Blt's result not looked at) and its back buffer
//   got. Otherwise: the window shown (9) and moved so that its client area is the view's, a primary, an off-screen surface and a clipper on the window, the masks of the primary into the view, the window placed and
//   shown (1). Then the mode's masks to the view (Surface_SetMasks). Every call whose result is kept stores it at 0x879cd4 (d3dresult.h), and anything but 0 fails.
u32 __fastcall DirectDraw_Open(u8 *object, void *, s32 width, s32 height, u32 depth, u32 driver, u32 fullScreen, u32 videoMemory);
}
