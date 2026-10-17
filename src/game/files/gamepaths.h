// The game's folder paths, built from the five path strings of the object at 0xf816c0 (Strings5; the first is the game's root folder):
// reimplementation of the fonts folder's path (0x468250).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x468250 (cdecl; out): with no text in the root folder's string (Obj_Self of 0xf816c0, its data at +8) 0; else out = the root and "Data\Fonts"
// (0x74d6c0) by sprintf, "%s%s" (0x74d320) when the root's last char is "\" and "%s\%s" (0x7488ac) when not (an empty root: the byte before its
// text read, sic), and 1.
u32 __cdecl GamePaths_FontsFolder(char *out);

// 0x4688b0 (cdecl; low; name unconfirmed): the two texture folder names by sprintf: 0x7af650 = "stexture" (0x74d6f4) and 0x7af720 = "ssharete" (0x747e88)
// when low is not 0, else "textures" (0x74d6e8) and "sharetex" (0x74d6dc). Returns the second sprintf's count.
s32 __cdecl GamePaths_SetTextureFolders(u32 low);
}  // extern "C"
