// A level's terrain texture sets file (Terrain\<name>.tct): reimplementation of 0x427620.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x427620 (cdecl (name, sets); name unconfirmed): the sets (gTerrainSets 0x74c8a8 for its one caller; Model_AddTerrainSets's layout:
// +0 the group count, +4 the groups (12 bytes: +0 name, +4 tile count, +8 tiles of 12 bytes: +0 number, +4 name), +8 the tiles, +0xc the
// bytes they need, +0x10 the block holding them, +0x14 the tiles placed) read from the text file in two passes, each reading it whole
// (VFS_Open; none: the pass does nothing more) into a zeroed GlobalMem block of its size (freed after). The second pass first gets
// the block of +0xc bytes (+0x10; +8 the tiles counted, +0x14 0) and gives each group its share of tiles in turn, the names following
// them; both clear the groups' counts. The text (TextScan) is lines: from '#' or ';' a comment to the line's end; a name (a letter, then
// letters and digits, up to 255) makes the group of that name (CRT _stricmp; none: none) current; a number (up to 11 digits or
// punctuation, CRT atoi) and the next token (letters, digits, punctuation, up to 255) a tile of the current group: the first pass adds
// 0xc + its length + 1 bytes to +0xc (which it never clears) and counts it, the second places it (number, the name copied after the
// last) and counts +0x14; either counts it in the group. Other punctuation skips the line; other characters are passed over. Returns 1.
u32 __cdecl TerrainSets_Load(const char *name, u8 *sets);
}
