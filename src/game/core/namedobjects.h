// The game's registry of named objects (the {list, cached} pair at [0x786b9c]; nodes {object, next, prev}, each object named by its String at +4):
// reimplementation of 0x408f00, and of the constructor and destructors of the registry's owner (the global 0x786b68: a String, its name, at +0,
// +0x14 -1, +0x18, +0x1c, the byte +0x20, a list of owned objects at +0x24 (vtable, head, tail, count; nodes {object, next, prev}), the
// registry (a {list, cached} pair, new) at +0x34 (so [0x786b9c]), +0x38 and +0x3c 1).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x408f00 (thiscall at its one call site, but the object is not used; name unconfirmed): for each of the 14 named globals 0x786b68, 0x785e78, 0x785f98,
// 0x7862c8, 0x785f68, 0x7862f8, 0x786d10, 0x7866e8, 0x7862e0, 0x785ed8, 0x785f80, 0x786aa8, 0x786040 and 0x7867d8 in turn, the first registered object with
// its name (NamedList_Find) is removed and deleted (ListRef_Remove), the registry re-read from 0x786b9c for each call. Returns the last call's result.
u32 __cdecl NamedObjects_RemoveGlobals();

// 0x407b90 (thiscall; name unconfirmed): String_Ctor, the fields above set (the list empty with vtable 0x6fd704), the name assigned from
// the global WString 0x7861c0 (the String vtable's +0x18, String_AssignWide), the registry a new 0xc-byte block cleared by Init_421510
// (0 when the allocation fails); returns the object.
u8 *__fastcall NamedObjects_Ctor(u8 *self);

// 0x407c90 (thiscall; name unconfirmed): while the list's count is above 0, its head node is unlinked and freed (the count lowered) and its
// object, if any, deleted (its vtable's +0xc with 1); a null head leaves the count as it is (sic: the loop never ends). Then the registry is
// drained (ListRef_DrainAndDelete) and freed, the +0x38 block freed (its empty destructor left out), the list gets vtable 0x6fd708 and
// every node left is freed, the count cleared, and String_Dtor.
void __fastcall NamedObjects_Dtor(u8 *self);

// 0x407b80 (cdecl, at exit; name unconfirmed): NamedObjects_Dtor of the global 0x786b68.
void __cdecl NamedObjects_StaticDtor();

// 0x409620 (thiscall; name, a String; pops 4; name unconfirmed): a new String (Mem_Alloc 0x14, String_Ctor; none when the allocation
// fails) assigned the name (the String vtable's +0x18) goes into the owned list (+0x24), sorted case blind: in an empty one (count 0) as
// its head; nowhere if the list already holds that String (the new one left allocated); else before the first whose name the new one
// is not after (Names_FirstAfter): at the head through List_PushHead, in the middle linked before it, or at the tail when there is none.
// sic: the head's object is read before the head is tested, so a count with no head faults.
struct String;
void __fastcall NamedObjects_AddName(u8 *self, void *, const String *name);

// 0x409260 (thiscall on the registry's owner 0x786b68, through 0x40f220, once a frame): Obj_AdvanceUnless30 on the game clock 0x785970;
// 0x7477e4 = bit 3 of the flags 0x7a1a14; and when the owner's dword at +0x18 is set bit 1 of the flags is set. Returns the flags then,
// else 0.
u32 __fastcall Flags7a1a14_Update(const u8 *object);

// 0x409a30 (thiscall on the owner 0x786b68, which it does not use; theme, label; pops 8; name unconfirmed): the theme's wide name is compared
// case blind (WNames_Equal) with the globals 0x786478 "fantasy", 0x786808 "halloween", 0x786970 "jungle" and 0x7869e8 "space"; for the first
// that matches, the text of the record table [0x78bad4] at 2, 1, 0 or 3 (Records_DataIfTerminated), widened (Language_Widen), goes to the
// label's vtable +0x14. No match: nothing. An unused String is constructed and destroyed around it.
struct WString;
void __fastcall NamedObjects_SetThemeLabel(u8 *self, void *, const WString *theme, u8 *label);

// 0x4099b0 (thiscall; theme, a String, label; pops 8; name unconfirmed): the theme name widened into a local WString
// (WString_AssignFromAnsi); when that converts anything, NamedObjects_SetThemeLabel with it.
void __fastcall NamedObjects_SetThemeLabelAnsi(u8 *self, void *, const String *theme, u8 *label);

// 0x4088a0 (thiscall on the owner 0x786b68; root, a WString; pops 4; name unconfirmed): the 14 named objects of the globals in
// NamedObjects_RemoveGlobals' order are made and registered: for each a new block (Mem_Alloc 0x48 / 0x40 / 0x50) is constructed with
// the global as its name and a local String as its folder (0x41d8a0 for 0x786b68, 0x785f80, 0x786aa8, 0x786040 and 0x7867d8; 0x41d7c0
// for 0x785e78 and 0x785f98; 0x41db70 for 0x7862c8, 0x785f68, 0x7862f8, 0x786d10 and 0x7866e8; 0x41da70 for 0x7862e0; 0x41dfc0 for
// 0x785ed8; none when the allocation fails), given that folder and its WAD (Class6fd9d8_AddFolderAndWad, a null object too) and added
// to the registry at +0x34 (Obj_SetAt8AndAdd for the second and third, else ObserverList0_Add). The folder is the root (the String
// vtable's +0x18, String_AssignWide) for the first; before each later one it is assigned the root again, ends in "\"
// (String_EnsureTrailingBackslash) and gets the next of the Strings 0x7869d0, 0x7864f0, 0x7865f8, 0x786640, 0x786388, 0x785f38,
// 0x7868e0, 0x786778, 0x786af0, 0x7863b8, 0x786850, 0x7869a0 and 0x7869b8 appended (vtable +0x28).
void __fastcall NamedObjects_AddFolders(u8 *self, void *, const WString *root);

// 0x4081e0 (thiscall on the owner; name unconfirmed): for each node {name, next} of the list at [0x786b90] (the names MakeFolders makes
// folders for), three objects (Mem_Alloc 0x40, Class6fda68_Ctor; none when the allocation fails) are made and added to the registry at
// +0x34 (ObserverList0_Add). Each gets a folder String: the WString 0x785e48 assigned (the String vtable's +0x18), "\" ensured
// (String_EnsureTrailingBackslash), the name appended (vtable +0x28), "\" ensured, then 0x786430 / 0x785e90 / 0x786670 appended; and
// a name String: 0x786a48 / 0x786ce0 / 0x7860e8 assigned (+0x18), the node's name appended. The folder and its WAD are added to the first
// two (Class6fd9d8_AddFolderAndWad), also under the second root to the third (Class6fd9d8_AddFolderAndWadBoth), a null object too. Both
// Strings are made and destroyed per node.
void __fastcall NamedObjects_AddListedFolders(u8 *self);
}
