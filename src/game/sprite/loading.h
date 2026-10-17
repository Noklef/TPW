// The sprite files found while loading a sprite folder: reimplementation of 0x541210.
//
// Sprites_LoadFolder (0x541310) lists the folder's files into a table of 12-byte entries at
// 0x875970 (count 0x875430): {file object, dword, byte}, the file's name a char pointer at the
// object's +0x20.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x541210 (cdecl): bubble-sorts the table by name (Names_FirstAfter on String copies of the
// names: case blind), passes until one makes no swap. A swap exchanges the two dwords and the
// byte of the entries; their 3 padding bytes stay where they were.
void __cdecl SpriteFiles_Sort();
}
