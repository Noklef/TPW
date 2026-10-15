// The quickload indexes: reimplementation of 0x461790-0x461a5c (the loader, 0x461a60, is not
// yet ported).
//
// A qickload.txt (sic) lists the files a ride directory holds, so rides can be found without
// searching. There are two tables of [0x6fe228] (1) indexes of 12 bytes {count, names, the
// names' text}: the base directory's (0x7a45dc) and the second directory's (0x7a3e5c, under
// 0x7af650), picked by a flag. 0x7a3d3c says the second is in use, 0x7a445c that they have
// loaded. Names compare with the CRT's _stricmp.
#pragma once

#include "../../../re/types.h"

struct QuickloadIndex {
    u32 count;
    char **names;
    char *text;
};

extern "C" {

// 0x461790: allocates both tables (zeroed) and marks the second directory in use.
void __cdecl Quickload_Init();

// 0x4617e0: frees both tables (not the names) and clears both marks.
void __cdecl Quickload_Free();

// 0x461820: 1 if index's list (second == the second directory's) has name, 0 if not; -1 when
// nothing has loaded.
s32 __cdecl Quickload_Has(const char *name, u32 second, u32 index);

// 0x4618a0: the first index whose list has name, or -1 (also when nothing has loaded).
s32 __cdecl Quickload_Find(const char *name, u32 second);

// 0x461920: Quickload_Has, comparing names without their extensions (cut at the last '.'
// unless a '\' follows it), through 0x104-byte stack copies (a longer name overruns, sic). 0
// when nothing has loaded. Note the argument order.
u32 __cdecl Quickload_HasStem(const char *path, u32 index, u32 second);
}

