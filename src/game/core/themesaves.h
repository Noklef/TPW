// The theme's own save file: reimplementation of 0x40bf90, 0x40c040 and 0x550b30. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "addrcalls.h"

// Calls by the game's address (the bench's recorders): UserSaves_LoadA, _SaveA, _SaveB, UserSlots_ThemeFolder and the local file stream's calls.
void ThemeSaves_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x40bf90 (cdecl): the theme's label (the String 0x786b68, copied and widened) saved (UserSaves_SaveA on 0xf7b560). Returns 1.
u32 __cdecl ThemeSaves_Save();

// 0x40c040 (cdecl): the same, loaded (UserSaves_LoadA). Returns 1.
u32 __cdecl ThemeSaves_Load();

// 0x550b30 (cdecl): the path UserSlots_ThemeFolder of UserSlots_Instance and the wide strings 0x878f18, 0x878a68, 0x877f38, made narrow, opened by a local
// file stream (FileStream_OpenPath, mode 0x80000000): no file: UserSaves_SaveB of 0x878a68; a file: the stream closed. The strings and the stream destroyed.
void __cdecl ThemeSaves_EnsureFile();
}
