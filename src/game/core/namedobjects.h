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
}
