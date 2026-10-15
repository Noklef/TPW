// The texture directories: up to three paths (each ending in a backslash) the texture loader
// looks in, kept as a table of string pointers (0x7b062c, 0x7b0630 entries) at indices
// 0x74d980-0x74d988 (-1: none). Each change allocates a new GlobalMem block (chained from
// 0x7b0628 by its first word) for the table and the paths that changed; unchanged paths keep
// pointing into older blocks. Reimplementation of 0x46e610-0x46e640, 0x46e670 and 0x46ea80.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x46e610: the first directory (0x74d980's), or null when there is none.
const char *__cdecl TexDirs_First();

// 0x46e630: the pointer at 0x7b0620 (the model loader sets it, 0x461dff).
u32 __cdecl TexDirs_Current();

// 0x46e640: sets it.
void __cdecl TexDirs_SetCurrent(u32 current);

// 0x46e670: the directories base + a, b and c (null: none), with a backslash appended when the
// name does not end in one. sic: an empty name's check reads the byte before it. A path equal
// to the one at its index before keeps that string; the rest are copied after the new table,
// which is only allocated when one of them changed (else the indices are written into the old
// table).
void __cdecl TexDirs_Set(const char *base, const char *a, const char *b, const char *c);

// 0x46ea80: frees the chain of blocks and clears the table, its count and 0x7b0620; no
// directories.
void __cdecl TexDirs_Reset();
}
