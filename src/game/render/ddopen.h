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

// 0x564160 (thiscall; name invented, unconfirmed): [0x8bd524] 1: DirectDraw_Open with [0x8bcbd0], [0x8bcbd4], [0x8bcbe4], +0x78c, bit 0 of +0x780 and 1, and its answer. Else Restore (slot 0x6c) of the surfaces at +0x10 and +0x14: 1 when both gave 0, stopping at the first that did not.
u32 __fastcall Display_RestoreOrOpen(u8 *self);
}

struct AddrCall;
void Display_SetRestoreCalls(const AddrCall *table, u32 count);   // tests/fncmp/displayrestore.py

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[28], gText_Ddopen_76628c, 0x0076628c, 0x1c);
GAME_DATA(char[36], gText_Ddopen_7662a8, 0x007662a8, 0x24);
GAME_DATA(char[44], gText_Ddopen_7662cc, 0x007662cc, 0x2c);
GAME_DATA(char[24], gText_DirectDraw_Open_7662f8, 0x007662f8, 0x18);
GAME_DATA(char[36], gText_DirectDraw_Open_766310, 0x00766310, 0x24);
GAME_DATA(u32, gDdopen_879cd4, 0x00879cd4, 0x4);
GAME_DATA(u32, gDdopen_8bd4c4, 0x008bd4c4, 0x4);
GAME_DATA(u32, gDdopen_8bd4cc, 0x008bd4cc, 0x4);
GAME_DATA(u32, gDdopen_8bd4d4, 0x008bd4d4, 0x4);
GAME_DATA(u32, gDdopen_8bd4dc, 0x008bd4dc, 0x4);
