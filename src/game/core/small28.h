// Searches of a list of named objects and the destructor of five strings: reimplementation of 0x421770, 0x41ede0 and 0x5c4c60. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x421770 (thiscall on {list, cached}; key, out, pops 8): the first object of the list (+4 head, nodes {object, next}) whose name (Names_Equal on object + 4) equals the key
// (an object too) is stored to *out and to the cached pointer at +4, and 1 is returned; otherwise (or without a list) the cached pointer is cleared and 0 returned.
u32 __fastcall NamedList_Find(u8 **ref, void *, const u8 *key, u8 **out);

// 0x41ede0 (thiscall; key, pops 4): whether an object of the list the dword at +0x4c points to has a name equal to the key's (any non-zero result of Names_Equal).
u32 __fastcall Obj_HasNamedObject(u8 *object, void *, const u8 *key);

// 0x5c4c60 (thiscall): the five strings at +0x50, +0x3c, +0x28, +0x14 and +0 are destroyed in that order (String_Dtor).
void __fastcall Strings5_Dtor(u8 *object);

}  // extern "C"
