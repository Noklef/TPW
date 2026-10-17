// The game's registry of named objects (the {list, cached} pair at [0x786b9c]; nodes {object, next, prev}, each object named by its String at +4):
// reimplementation of 0x408f00.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x408f00 (thiscall at its one call site, but the object is not used; name unconfirmed): for each of the 14 named globals 0x786b68, 0x785e78, 0x785f98,
// 0x7862c8, 0x785f68, 0x7862f8, 0x786d10, 0x7866e8, 0x7862e0, 0x785ed8, 0x785f80, 0x786aa8, 0x786040 and 0x7867d8 in turn, the first registered object with
// its name (NamedList_Find) is removed and deleted (ListRef_Remove), the registry re-read from 0x786b9c for each call. Returns the last call's result.
u32 __cdecl NamedObjects_RemoveGlobals();
}
