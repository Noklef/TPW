// The saves in the chosen user's theme folder: reimplementation of 0x5ac5d0-0x5ac8eb (one save loaded or saved by name), 0x5ac8f0 (the folder's saves listed) and 0x5accf0 (the
// newest loaded). All names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"
#include "../core/string.h"

// Calls by the game's address (the bench's recorders): the local handle's Handle_Ctor, Handle_Open28, Handle_Vfn1, Handle_Vfn2, Handle_Dtor, UserSlots_ThemeFolder, 0x5c8a10, SaveGame_Load and
// SaveGame_Save.
void UserSaves_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x5ac650 (stdcall; name, extension, pops 8): the path UserSlots_ThemeFolder of UserSlots_Instance, the global wide "\" (0xf7b888), name and extension, made narrow (String_AssignFromWide), goes to
// SaveGame_Load of the game's save object 0x78a450 (no header, mode 2). Returns its answer.
u32 __stdcall UserSaves_Load(const WString *name, const WString *extension);

// 0x5ac780 (stdcall; name, extension, pops 8): 0x5c8a10 on UserSlots_Instance, then the same path to SaveGame_Save. Returns its answer.
u32 __stdcall UserSaves_Save(const WString *name, const WString *extension);

// 0x5ac5d0, 0x5ac5f0 (stdcall; name, pops 4; ECX unused): UserSaves_Load with the wide globals 0xf7ad98, 0xf7adf8 as the extension.
u32 __stdcall UserSaves_LoadA(const WString *name);
u32 __stdcall UserSaves_LoadB(const WString *name);

// 0x5ac610, 0x5ac630 (stdcall; name, pops 4; ECX unused): UserSaves_Save with the same two.
u32 __stdcall UserSaves_SaveA(const WString *name);
u32 __stdcall UserSaves_SaveB(const WString *name);

// 0x5ac8f0 (thiscall on a list: +4 head, +8 tail, +0xc count; nodes {item, next, prev}): the list emptied (as many items as its count destroyed and freed, then any nodes left freed). The theme folder
// opened as a folder source (Handle_Open28; vtable 0x6fd9e8) and searched for the String 0xf7b948 + the String 0xf7b548: each entry not a real directory (DirEntry_IsRealDir) makes an item of 0x1c
// bytes, a wide string of its name without as many last chars as 0xf7b548 has, its +0x14 and +0x18 the entry's write time, appended to the list. Returns the first item, 0 for none.
u8 *__fastcall UserSaves_List(u8 *list);

// 0x5accf0 (cdecl): the theme folder searched for the String 0xf7b948 + the String 0xf7b398: of the entries not real directories, the first with the latest write time (FileTime_Compare, the later
// one only if above), loaded by SaveGame_Load (no header, mode 2) from the folder, the String 0xf7aed0 and its name. Nothing found: nothing loaded.
void __cdecl UserSaves_LoadNewest();
}
