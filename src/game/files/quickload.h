// The quickload indexes: reimplementation of 0x461790-0x461a5c and of the loader 0x461a60,
// and of 0x44a280, a ride file located through them or the file system.
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

// 0x44a280 (cdecl; name, flags, &index or 0; name unconfirmed): 0 for a null name. Without loaded indexes flag 2 is dropped. With
// flag 2 the name's last part (after its last '\') is looked up (Quickload_Find, the second directory's with flag 8) and the
// index, -1 too, stored; -1 fails (0) unless flag 4, which drops flag 2. Without flag 2 the index stored is -1. The name is then
// copied to the buffer at 0x79c9d0 (after the directory [0x79cb4c][found with flag 2, else 0] is read). With flag 2 that is the
// answer; else the copy must open (VFS_FileExists: the index stored is 0) or the index stored is -1 and 0 returned. The answer is
// the buffer with flag 1, else the directory read.
char *__cdecl Quickload_Locate(const char *name, u32 flags, s32 *index);

// 0x461a60 (cdecl; name, folder, c; name unconfirmed): a ride's quickload indexes read and its texture directories set. Loaded
// indexes first freed (both tables' names and text, from the last index down) and the mark cleared. The located path "folder\name"
// (with no folder the name alone, as the format, sic) goes to Quickload_Locate (flags 0); index -1 fails (0). With the second
// directory in use (0x7a3d3c) the two qickload.txt files ("base\folder\qickload.txt", then "base\folder\second\qickload.txt",
// base [0x79cb4c][index], second the text at 0x7af650) are read whole through the VFS (slot 0x14 the size, a buffer of that from
// GlobalMem_Alloc, slot 8 the read) into the base table's and the second table's entry index: emptied first; a file read sets the
// loaded mark and its words (TextScan_Read of every class, blanks between them) are counted, then stored in one text buffer with
// a pointer to each. Then the current texture directory index set, the directories "folder" + "\" (unless folder ends with one;
// an empty folder looks at the byte before it, the located path's last, sic) + the second text, and with the directory Locate
// returned TexDirs_Set(directory, folder, those directories, c) and 1; else 0.
u32 __cdecl Quickload_LoadRide(const char *name, const char *folder, const char *c);
}
