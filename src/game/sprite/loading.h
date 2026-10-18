// The sprite files found while loading a sprite folder: reimplementation of 0x541210; and the freeing of the loaded sprites, 0x541ed0.
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

// 0x541ed0 (cdecl): each of the [0x87626c] loaded sprites (gSprites, 0x876b48; the count re-read each pass) with its frame set (the
// byte at +0x204 set) gets it freed: SpriteFrames_Free on the set at +0 (even a null one), then again on it and the set freed
// (GameHeap_Free) when it is not null, +0 cleared; then the sprite itself is freed when not null. Then the 16 draw list objects
// (0x8751f0, 0x24 bytes apart) and the overlay one (0x876420) are emptied (TrackedArray_Free).
void __cdecl Sprites_FreeAll();
}
