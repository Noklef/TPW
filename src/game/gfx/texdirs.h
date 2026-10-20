// The texture directories: up to three paths (each ending in a backslash) the texture loader
// looks in, kept as a table of string pointers (0x7b062c, 0x7b0630 entries) at indices
// 0x74d980-0x74d988 (-1: none). Each change allocates a new GlobalMem block (chained from
// 0x7b0628 by its first word) for the table and the paths that changed; unchanged paths keep
// pointing into older blocks. Reimplementation of 0x46e610-0x46e640, 0x46e670 and 0x46ea80, and of
// 0x46e4e0 and 0x46ead0, the directories set for a model and a texture set's textures given theirs.
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

// 0x46ead0 (cdecl; set, probe, a third argument not read): nothing for a null set. The set takes
// the table (+0x4c) and its count (+0x34, a word); then each of its textures (+0x36 of them, a
// word read each time, 0x10 bytes each from [+0x54]: +0xc the name, +4 the directory got) is
// looked for under the second directory (0x74d984's): with the quickload indexes loaded (and
// probe 0) by Quickload_HasStem(name, [0x7b0620] as read before, 1), else as the file second +
// name with its extension (from the last '.', or appended) ".wct", then ".tga" (VFS_FileExists).
// Found: the set's table at 0x74d984's index (both read then). Not found: the third directory's
// entry likewise when there is one, else the table's first. The game's "Could not load texture"
// messages (_snprintf to a stack buffer never read) are left out.
void __cdecl TextureSet_Load(u8 *set, u32 probe, u32 unused);

// 0x46e4e0 (cdecl; info, model; name unconfirmed): the file name [info+8] + (a backslash when
// that does not end in one; sic: an empty one's check reads the byte before) + [[info+0x10]+4],
// on the second try with its extension (as above) ".wct", located (Quickload_Locate(path, 0, 0):
// the quickload directory if it exists); not found twice, the path itself. The directories are
// set to that + [info+8] and that + [info+0xc] (TexDirs_Set(that, 0, ...)) and the model's
// textures, [[model+0x24]+4], found with probe 1.
void __cdecl TexDirs_SetForModel(u8 *info, u8 *model);
}
