// The game's folder paths, built from the five path strings of the object at 0xf816c0 (Strings5; the first is the game's root folder):
// reimplementation of the fonts folder's path (0x468250), the save folder's (0x5c7bd0) and its users and online folders' (0x5c7c70, 0x5c8980),
// and the folders made under a root (0x5c7ac0).
#pragma once

#include "../../../re/types.h"
#include "../core/string.h"

struct WString;

extern "C" {

// 0x5c7ac0 (thiscall, this unused; root; pops 4; name unconfirmed): for each node of the list at [0x786b90] (+0 a name String, +4 the next),
// the root (a wide String, String_AssignFromWide), the String at 0xf81b48 and the name appended (String_Append): when that folder does
// not open (a folder handle, vtable 0x6fd9e8, Handle_Open28) it is created (Dir_Create).
void __fastcall GamePaths_MakeFolders(void *, void *, const WString *root);

// 0x5aa650 (cdecl; a String by value, destroyed at the end; name unconfirmed): a local String given the drive (vtable +0x18, assign), ":" (0x77316c,
// +0x20, append a C string) and the String at 0xf79fe8 (+0x28, append) is assigned to the second path string of 0xf816c0 (Obj_CallSub14Slot18:
// its +0x14's vtable +0x18); a second local String is made and destroyed unused. Its caller (in 0x5aa4xx's drive scan) passes a drive's letter.
void __cdecl GamePaths_SetSecondRoot(String drive);

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
}  // extern "C"
