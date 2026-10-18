// Tests on a directory entry (the DirEntry a source's FindFirst / FindNext fills): reimplementation of 0x5c53c0 and 0x5c5230.
#pragma once

#include "../../../re/types.h"
#include "dirsource.h"

extern "C" {
// 0x5c53c0 (cdecl; entry; name unconfirmed): a String of the entry's name (String_CtorStr), and a copy of its last chars, as many
// as the global ".wad" String at 0xf80808 has (String_Right, String_CtorCopy), compared with that global (Names_Equal). 1 when equal
// (any non-zero result), else 0.
u32 __cdecl DirEntry_IsWad(const DirEntry *entry);

// 0x5c5230 (cdecl; entry): 1 for a directory entry (bit 0x10 of the byte at +4) whose name (the string at +0x20)
// does not start with a full stop, else 0.
u32 __cdecl DirEntry_IsRealDir(const u8 *entry);
}
