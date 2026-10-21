// The park map's squares classified (0x5f2050; names unconfirmed, the map seemingly the park overview drawn next by 0x5f3710): thiscall on the
// map, whose width (+0x24) and height (+0x28) cover the map cells from (0, 0); each square (x, y) has an 8-byte entry at +0x60 + (y * 0x80 + x)
// * 8: a kind (dword) and a thing handle (word at +4), both cleared first. A square out of range (x or y not under 0x80) writes to address 0
// instead (sic).
//
// The square's 0x44-byte map cell is [0x7cf83c] + 0x2d8 + (id - 1) * 0x44, id = (y << 7) + x + 1 cut to 16 bits. Its kind is
// - 9 when its 0x28-byte entry (or a 0xc / 0x11 entry's parent) is of type 0x19 (MapHandle_ParentIsType19);
// - for a cell of type 4, 9, 0xa or 0x15 or a track one (MapHandle_IsTypeBDOr10Or12Or19), the first thing on the owner's thing list that tells
//   one (below), else 0. The owner is the cell's +0x10 word, or for a track one the +0x10 word of its entry (MapHandle_Entry28), or of the entry's
//   parent ([0x7cf83c] + (its +0x10 + 0x6cde) * 0x28) for a 0xc / 0x11 one. The owner's 0x44-byte cell's word at +0x24 heads the list, each
//   thing (the thing table at 0x7cfb90, 20 bytes a slot) linked to the next by its word at +0xa. A thing of model 3 (+2) whose definition
//   (Object_GetDef) has a map kind (+0x4ac) of 0 to 3 tells: 0 gives 1 (8 for a track or type 0x15 square that is not of type 4), 1 gives 2,
//   2 gives 3, 3 by its flags byte (+0x32) 5 (bit 0 or 6), 6 (bit 4), 7 (bit 1) or 4; its handle is kept too;
// - else 0xa for type 1, 0xb for type 3 (MapCell_IsType3Or9: 9 never gets there), 0xc with flag 0x40 (MapCell_HasFlags), or 0.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x5f2050 (thiscall on the map).
void __fastcall ParkMap_Classify(u8 *map);
// 0x5f4180 (thiscall on the map; sprite, pops 4; name unconfirmed): a sprite (its frame at +0x28, as TexSprite's) placed on the overview
// from its map position (+4, +8): screen x, y = (pos - the view origin (+0x54, +0x58)) * 480 / width (425 / height, rounded to a float) *
// the zoom (+0x5c) + half the screen's width (height) (0x7a1968 / 0x7a1930), stored at +0xc / +0x10; the frame's vector in the array at
// 0x879ce0 (PtrArray_SetVectorOf) set to (2x / screen width - 1, 2y / screen height - 1, 0.99 for a layer (+0x14) of 0 to 2, else
// 0.9999), and the frame's +0x2c = the word at +0xc of the frame's +0x28 times the sprite's size (+0x24) * zoom * 0.5 / 16. x87 at the
// game's precision (X87Reg); x is reread as the float stored.
void __fastcall ParkMap_PlaceSprite(const u8 *map, void *, u8 *sprite);
// 0x5f1050 (thiscall on the map; name unconfirmed): the view origin (+0x54, +0x58) kept to 0 to width - 1 (height - 1; a NaN or negative
// one to 0, one past the limit to the limit, compared as an integer) and stored back as floats; the camera put on it (Camera_SetView of x and z
// each truncated (the limit unrounded) times 10 as floats), then the map's window (+4) sent message 4 (0, 0). Returns what that returns.
s32 __fastcall ParkMap_ApplyView(u8 *map);
// 0x5f3230 (thiscall on the map; name unconfirmed): the overview's backdrop (0x87aee8) framed on the view, then the map's sprites placed. The
// centre is (480 / width (+0x24) * the origin x (+0x54) + 15, 425 / height (+0x28) * the origin y (+0x58) + 42), the half sizes the screen's
// (0x7a1968, 0x7a1930) / the zoom (+0x5c) / 2: left and top (below 0, or a NaN: 0; the left tested before its store) and right and bottom
// (from the centre as stored, a float; from width - 1 (+0x48) and height - 1 (+0x4c) as unsigned on: that limit) are each truncated (__ftol) and given as floats to
// Backdrop_SetRect; the source rect (Backdrop_SetSourceRect) is the screen half size less / plus the centre's distance from each edge times
// the zoom, doubled, over the screen size, less 1, each kept to -1 to 1 (a NaN to -1). Then each sprite on the list at +0x20080 (its +4 the
// first node: sprite, next) is placed (ParkMap_PlaceSprite). x87 at the game's precision (X87Reg), floats where the original stores them.
void __fastcall ParkMap_LayOut(const u8 *map);

// 0x5f35b0 (thiscall on the map; direction, amount, pops 8; name unconfirmed): the view origin moved: direction 0 x (+0x54) less the amount,
// 1 x plus it, 2 y (+0x58) plus it, 3 y less it (any other: neither), each stored as a float; then each kept to [0x703010] (below it or a
// NaN: it) to width - 1 (+0x24; height - 1, +0x28: signed, compared as the integer, stored rounded to a float) and the overview laid out
// again (ParkMap_LayOut). x87 at the game's precision (X87Reg).
void __fastcall ParkMap_Scroll(u8 *map, void *, u32 direction, float amount);

// 0x5f3940 (thiscall on the park map; name unconfirmed): its sprites added. For each thing in use (World_FirstThing / World_NextThing, the
// world at 0x7cf83c) by its kind (+2): 1 a sprite of image +0x2006c (TexSprite_Ctor 1, 1, 0x20, 1.0f) and one of +0x20068 (0, 3, 1, 1.0f);
// 4 / 5 one of +0x20070 (2, 2, 2 / 1, 1.0f); 6 / 7 one of +0x20070 (2, 2, 4 / 3, 1.0f) and one of +0x20078 / +0x2007c (4 / 5, 2, 4 / 3, the
// int at 0x785610 / 0x785658 times 0x703030's float). Each is a new 0x30-byte block (Mem_Alloc; a failed one faults, sic) at the thing
// (+0x18) placed (ParkMap_PlaceSprite) at its bytes +5 / +7 less +4 / +6 times 0x703064's float. Then for each square of the map's width
// (+0x24) x height (+0x28, signed) whose cell's kind (+0x2c) is set, and for each thing on the list from the world's u16 +0x1da746 (linked
// by +0xc) whose flags +0x32 have 0x10, a sprite (TexSprite_Ctor inlined: +0x2c 1 first, then cleared through Obj_SetShown) of image
// +0x20074 (3, 2, 1; the size 1, or the int at 0x785688 / 0x785640 for kinds 8 / 7, times 0x703030's float) / +0x2007c (5, 2, 3; 0x785670's
// int times it), +0x18 0, placed at the square / the thing's map square (Thing_MapId) less 0x703028's float. x87 at the game's precision.
void __fastcall ParkMap_AddSprites(u8 *map);
}
