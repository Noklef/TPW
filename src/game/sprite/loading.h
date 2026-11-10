// The sprite files found while loading a sprite folder: reimplementation of 0x541210; and the freeing of the loaded sprites, 0x541ed0.
//
// Sprites_LoadFolder (0x541310) lists the folder's files into a table of 12-byte entries at
// 0x875970 (count 0x875430): {file object, dword, byte}, the file's name a char pointer at the
// object's +0x20.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

struct String;

extern "C" {

// 0x541310 (cdecl; folder, name; name from the table in symbols.csv): the sprites of a folder loaded. The byte 0x877354 set. A name "kids" or
// "kidsheads" (case blind) takes the graphics level (Graphics_Level on 0x78baec) as a limit for the avatars, one of "handymen", "mechanics",
// "guards", "researchers" the graphics tier (Graphics_Tier) as another. The 0x80 entries of the file table cleared. Twice, the buffer 0x876668
// and a local String told the path: "generic\<name>\" then the wide string 0x786b68's data read as chars, "\", the name, "\". The String gets
// the String 0x8757f0 appended and its files are listed (Class6fd9d8_FindFile == 1, Class6fd9d8_Find into 0x875038): under 0x80 entries each gets
// a block (0x128 bytes: 0x20 of the found record, its name at +0x24 and +0x20 the name's pointer moved with it; it dies with a null block) and a new
// Handle (null when the allocation fails) told the folder's current object (Get34_41ce80, Handle_Assign). Sorted (SpriteFiles_Sort). With the
// avatar limit, each of the names gAvatarSprites 0x764030 in turn: the entries of that name not loaded yet. Then every other entry. Each loaded
// is a new sprite (0x260 bytes, Sprite_Ctor) in gSprites[[0x87626c]] with Sprite_LoadEsp(file, handle): its low byte set counts it and marks
// the entry, and the 0x200 sprites (the index as it now stands) or a limit (the avatars: only theirs; afterwards either that is on) ends the
// loading; else the sprite in that place is freed. In the end every entry's block is freed, its handle deleted (vtable +0x2c, 1), +0 cleared.
// Answers the count loaded.
u32 __cdecl Sprites_LoadFolder(u8 *folder, const String *name);

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

struct StreamBase;

// The calls of Sprites_LoadFolder go to these (the game's own by default; the bench points them at recorders).
void SpriteLoading_SetCalls(const AddrCall *table, u32 count);

// What the frame-set reloads call in the game, so that they can run outside it (the benches point these at recorders; the ports, or the
// game's own functions for those not ported, by default).
struct SpriteLoadingEnv {
    u32(__fastcall *find)(u8 **ref, void *, const u8 *key, u8 **out);                // NamedList_Find (0x421770)
    u32(__fastcall *framesFree)(u8 *set);                                             // SpriteFrames_Free (0x588230)
    u32(__fastcall *open)(u8 *folder, void *, StreamBase *stream, String *name);      // 0x41e740 (not ported; the game's)
    u32(__fastcall *load)(u8 *set, void *, StreamBase *stream, u32 zero);             // the frame set's load (0x587db0)
    void(__fastcall *markTextures)(u8 *set);                                          // SpriteFrames_MarkTextures (0x5882e0)
};
// The reloads' calls go to env (nullptr: the game's).
void SpriteLoading_SetEnv(const SpriteLoadingEnv *env);

extern "C" {

// 0x542420 / 0x542640 (cdecl; return a byte): the loaded sprites' frame sets swapped for the ground (.fpc) / park (.tpc) ones. Nothing
// (0) when the frames in use (0x763f80) are those already (0 ground, 1 park); else they become them, and the suffix is the wide string
// 0x875fe8 / 0x876270 when the byte at 0x763f28 is set, else 0x875598 / 0x8757a8 (String_AssignWide). The folder is looked up by the
// key 0x876960 in the list [0x786b9c] (NamedList_Find; not found: 0). Then each of the 0x876b28 (signed, re-read) sprites (gSprites,
// 0x876b48) with the byte +0x205: its name (+4) and the suffix make the file's name, its frame set (+0) freed (SpriteFrames_Free, also
// a null one; then again and the block freed when set) and cleared; a new stream (8 bytes, StreamBase_Ctor; never freed, sic) opened on
// the name in the folder (0x41e740) and, when that answers 1, a new set (0x10 bytes, +0 and +0xc cleared) at +0 loaded from it
// (0x587db0): +0x204 = it answered 1, +0x208 = the set's count; with +0x207 SpriteFrames_MarkTextures. Answers 1.
u8 __cdecl Sprites_UseGroundFrames();
u8 __cdecl Sprites_UseParkFrames();

// 0x540bc0 (cdecl; name invented, unconfirmed): the atexit entry of the sprite folder name 0x875058 (its static initialiser 0x540ba0 points it
// at the buffer 0x87505c): nothing.
void __cdecl SpriteFolderName_AtExit();
}

// 0x541d90 (cdecl; name invented, unconfirmed): the sprites' count [0x87626c] 0, the folder 0x876960 found in [0x786b9c]'s list (not exactly 1: false). Each of the 14 sets of 0x763f88 (only the last, with the flag), 0xc bytes each: its +4 the count so far and its +8 Sprites_LoadFolder(the folder, its name +0). [0x876b28] the count. True.
extern "C" {
bool __cdecl Sprites_LoadAll(u8 onlyLast);

// 0x540900 (cdecl (mode, onlyLast); name invented, unconfirmed): 0x8bcbc8 gets 0x1000000; [0x763f84] 1, [0x877350], [0x876b24], [0x8bd4f0], [0x8bd4f4] 0, [0x8768fc] the mode's byte, [0x763f80] whether it is 0. Sprites_LoadAll(onlyLast). [0x87734c] 0; the map [[0x7a0854]+0x6c]: its size (+0x18, +0x1c)
// into [0x876898], [0x876894] and a quarter of each (truncated) plus 1 into [0x8762b4], [0x87614c]. For each row y and column x of 4 by 4: the 0x24-byte cell 0x8751f0 + (x + 4 y) * 0x24 gets TrackedArray_Alloc(0x800) into the table 0x875528 and +4 0; Grid_AddObject [0x87a6d0] of its rectangle (y, x times a
// quarter; the last row and column end at the map's size less 1) and the cell plus its vtable's +4, 0. The same array of 0x20 at 0x876420 into [0x87692c], +4 0, Renderer_Add of it plus its offset, 0x20. Returns 1 in AL.
bool __cdecl Sprites_Setup(u32 mode, u32 onlyLast);
}
