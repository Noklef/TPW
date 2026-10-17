// Tests on a directory entry (the DirEntry a source's FindFirst / FindNext fills): reimplementation of 0x5c53c0.
#pragma once

#include "../../../re/types.h"
#include "dirsource.h"

extern "C" {
// 0x5c53c0 (cdecl; entry; name unconfirmed): a String of the entry's name (String_CtorStr), and a copy of its last chars, as many
// as the global ".wad" String at 0xf80808 has (String_Right, String_CtorCopy), compared with that global (Names_Equal). 1 when equal
// (any non-zero result), else 0.
u32 __cdecl DirEntry_IsWad(const DirEntry *entry);
}
