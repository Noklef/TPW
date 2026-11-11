// The game's folder paths, built from the five path strings of the object at 0xf816c0 (Strings5; the first is the game's root folder):
// reimplementation of the fonts folder's path (0x468250), the save folder's (0x5c7bd0) and its users and online folders' (0x5c7c70, 0x5c8980),
// and the folders made under a root (0x5c7ac0).
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"
#include "../core/string.h"

struct WString;

extern "C" {

// 0x5c7ac0 (thiscall, this unused; root; pops 4; name unconfirmed): for each node of the list at [0x786b90] (+0 a name String, +4 the next),
// the root (a wide String, String_AssignFromWide), the String at 0xf81b48 and the name appended (String_Append): when that folder does
// not open (a folder handle, vtable 0x6fd9e8, Handle_Open28) it is created (Dir_Create).
void __fastcall GamePaths_MakeFolders(void *, void *, const WString *root);

// 0x5c8190 (stdcall; root, copy; name invented, unconfirmed): the root folder made (Dir_Create), then for each node as above its folder (the root, "\\" 0xf81b48, the name) made.
// With copy: the file (the third String of 0xf816c0, "\\", 0xf81980, "\\", the name, "\\", "easymode" 0x773cd0, 0xf825e0), when it exists, copied to the same name in that folder.
void __stdcall GamePaths_MakeFoldersCopying(const WString *root, u32 copy);

// 0x5c5250 (cdecl; path; name invented, unconfirmed): the folder opened (a folder handle, Handle_Open28) and, when it opens, searched for the pattern [0xf81650]: each entry not starting with
// '.', a folder (attribute 0x10) removed the same way (the path, "\\" 0xf80be0, its name), a file by Handle_Vfn9; then the folder itself (Dir_Remove).
void __cdecl GamePaths_RemoveTree(const String *path);

// 0x5aa650 (cdecl; a String by value, destroyed at the end; name unconfirmed): a local String given the drive (vtable +0x18, assign), ":" (0x77316c,
// +0x20, append a C string) and the String at 0xf79fe8 (+0x28, append) is assigned to the second path string of 0xf816c0 (Obj_CallSub14Slot18:
// its +0x14's vtable +0x18); a second local String is made and destroyed unused. Its caller (in 0x5aa4xx's drive scan) passes a drive's letter.
void __cdecl GamePaths_SetSecondRoot(String drive);

// 0x5aa500 (thiscall on the CD check, its only caller 0x45f84f: a StringList at +0, the system-info object at +0x10; name unconfirmed):
// a WString text (vtable +0x14) widened (Language_Widen) from record 9 of the table [0x78ba88] (Records_DataIfTerminated) and a caption
// from the String 0xf7a7c8 (WString_AssignFromAnsi). Until the drives of type mask 8 are listed into the list (SystemInfo_ListDrives
// with a status >= 0) a MessageBoxW (the import [0x6fd378], read once) over the app's window with text and caption, 0x15 (retry,
// cancel; the hand icon), asks again; IDCANCEL (2) returns 0. Listed: the letter of the volume labelled as [0xf7a7d0] (that String's data;
// SystemInfo_FindVolume) as a one-char String (String_CtorChar) to GamePaths_SetSecondRoot, and 1 returned. Both WStrings destroyed.
u32 __fastcall GamePaths_FindDisc(u8 *check);

// 0x468250 (cdecl; out): with no text in the root folder's string (Obj_Self of 0xf816c0, its data at +8) 0; else out = the root and "Data\Fonts"
// (0x74d6c0) by sprintf, "%s%s" (0x74d320) when the root's last char is "\" and "%s\%s" (0x7488ac) when not (an empty root: the byte before its
// text read, sic), and 1.
u32 __cdecl GamePaths_FontsFolder(char *out);

// 0x4688b0 (cdecl; low; name unconfirmed): the two texture folder names by sprintf: 0x7af650 = "stexture" (0x74d6f4) and 0x7af720 = "ssharete" (0x747e88)
// when low is not 0, else "textures" (0x74d6e8) and "sharetex" (0x74d6dc). Returns the second sprintf's count.
s32 __cdecl GamePaths_SetTextureFolders(u32 low);

// 0x5c7bd0 (stdcall; out, the ECX unused; name unconfirmed): an empty wide string (WString_Ctor) given the root folder's text (WString_AssignFromAnsi of
// Obj_Self of 0xf816c0) and the global wide "save" (0xf818d8) appended through its vtable's +0x28 (WString_Append); out is constructed as a copy of it
// (WString_CtorCopy) and it is destroyed. Returns out.
WString *__stdcall GamePaths_SaveFolder(WString *out);

// 0x5c7c70 (stdcall; out; name unconfirmed): the save folder (GamePaths_SaveFolder into a local) with the global wide "\" (0xf82508) and "users"
// (0xf81ff8) appended through the vtable's +0x28; out constructed as a copy, the local destroyed. Returns out.
WString *__stdcall GamePaths_UsersFolder(WString *out);

// 0x5c8980 (stdcall; out; name unconfirmed): the same with "online" (0xf820e8) for "users".
WString *__stdcall GamePaths_OnlineFolder(WString *out);

// 0x5c0c10 (cdecl; result; name invented, unconfirmed): at start-up, after the base folder is set: the paths object 0xf816c0's first String
// given the base folder (Dir_GetBase, as a String: Obj_CallSlot18, String's AssignWide on it (sic: read as a WString, the same layout)),
// its +0x28 String the String 0xf7eaf0 (Obj_CallSub28Slot18), its +0x3c the C string 0x747810 joined on (Strings5_JoinPath) and its +0x50
// the String 0xf7edf8 (Strings5_JoinPathAt50), each through a temporary String. *result = 0; returns result.
u32 *__cdecl GamePaths_SetRoots(u32 *result);

// GamePaths_ShareTexFolder's call to 0x40ef40 goes by the game's address; a table of replacements (tests/fncmp/sharetexpath.py gives it a recorder).
void GamePaths_SetCalls(const AddrCall *table, u32 count);

extern "C" {
// 0x40ef40 (cdecl; name unconfirmed): the static String at 0x788028 (made at the first call, guard bit 0 of 0x7887e4, its destructor 0x40eff0 registered with
// atexit) given the String at 0xf816e8, a "\" (asked twice, sic), the String at 0x787f50, a "\" and the String of 0x786b68: its text is returned.
const char *__cdecl GamePaths_Base();
}

// 0x46e650 (cdecl; out; name invented, unconfirmed): sprintf(out, "%s\\%s", 0x40ef40's text, the text at 0x7af720). Returns sprintf's result.
s32 __cdecl GamePaths_ShareTexFolder(char *out);
}  // extern "C"

// 0x45f7a0 (cdecl; name invented, unconfirmed): the disc looked for. The statics at 0xfb34e0 (a string list, 0xfb34f0 the system's information) made the first time (guard bit 0 of [0xfb34ca]; its end with atexit). GamePaths_FindDisc on a copy of them (0x35 bytes); the copy's list freed, node by node (+0x14 the next). True when it answered exactly 1.
extern "C" {
bool __cdecl GamePaths_CheckDisc();
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[8], gText_Gamepaths_74d320, 0x0074d320, 0x8);
GAME_DATA(char[12], gText_GamePaths_FontsFolder_74d6c0, 0x0074d6c0, 0xc);
GAME_DATA(char[12], gText_GamePaths_SetTextureFolders_74d6dc, 0x0074d6dc, 0xc);
GAME_DATA(char[12], gText_GamePaths_SetTextureFolders_74d6e8, 0x0074d6e8, 0xc);
GAME_DATA(char[12], gText_GamePaths_SetTextureFolders_74d6f4, 0x0074d6f4, 0xc);
GAME_DATA(u16[2], gText_GamePaths_SetSecondRoot_77316c, 0x0077316c, 0x4);
GAME_DATA(char[12], gText_GamePaths_MakeFoldersCopying_773cd0, 0x00773cd0, 0xc);
GAME_DATA(u32[5], gGamePaths_Base_788028, 0x00788028, 0x14);   // a String (its text pointer at +8): one object
GAME_DATA(u8, gGamePaths_Base_7887e4, 0x007887e4, 0x1);
GAME_DATA(char[16], gGamePaths_7af720, 0x007af720, 0x10);
GAME_DATA(u8, gGamePaths_CheckDisc_fb34ca, 0x00fb34ca, 0x1);
GAME_DATA(u32[9], gGamePaths_CheckDisc_fb34f0, 0x00fb34f0, 0x24);
