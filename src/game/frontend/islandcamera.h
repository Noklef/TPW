// The lobby island's camera settings (the 0x6c-byte object of vtable 0x7029fc, made at 0x5d5a54): reimplementation of 0x5e2cc0, which reads
// them from a text file. +4 the field of view, +8 the spin speed, +0xc the spin radius, +0x10 the vertical offset, +0x14 / +0x18 the globe's
// outer and inner radius, +0x1c and +0x44 ten pairs of camera positions. Names from the file's keys; the class is unnamed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5e2cc0 (thiscall; name unconfirmed): the file named by the String at 0xf855b8 with the one at 0xf855d0 appended (String_Append, the
// String vtable's +0x28) opened (VFS_Open; the game also pushes the mode "r", which it does not read; a file that does not open faults,
// sic), its size taken (a seek to the end, slot 0x14, a seek back), read whole into a Mem_GlobalAlloc block and the stream finished;
// every '\n' made a NUL. Then for each line (from the block's start, each past the NUL that ends the one before, while below the end): a
// line starting (strncmp) "ISLANDFOV(", "SPINSPEED(", "SPINRADIUS(", "VERTICALOFFSET(", "GLOBERADIUSOUT(" or "GLOBERADIUSIN(" sets +4 ..
// +0x18 to atof of the text after its first '(' up to ')' (String_ReadToken); "ISLANDCAMERAPOSITION(" reads an index (atoi, to ','),
// two floats (to ',' and ')'), stored at +0x1c / +0x44 + 4 index when the index is 0-9. The '(' is searched for without bound, and a
// last line without '\n' is read on past the block to a NUL (sic). The block freed (Mem_GlobalFree).
void __fastcall IslandCamera_LoadSettings(u8 *settings);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0x18], gText_IslandCamera_LoadSettings_774ce0, 0x00774ce0, 0x18);
GAME_DATA(char[0x10], gText_IslandCamera_LoadSettings_774cf8, 0x00774cf8, 0x10);
GAME_DATA(char[0x10], gText_IslandCamera_LoadSettings_774d08, 0x00774d08, 0x10);
GAME_DATA(char[16], gIslandCamera_LoadSettings_774d18, 0x00774d18, 0x10);
GAME_DATA(char[12], gIslandCamera_LoadSettings_774d28, 0x00774d28, 0xc);
GAME_DATA(char[11], gIslandCamera_LoadSettings_774d34, 0x00774d34, 0xb);
GAME_DATA(char[11], gIslandCamera_LoadSettings_774d40, 0x00774d40, 0xb);
