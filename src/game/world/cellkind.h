// A map cell's kind (+8) changed and a cell cleared (names invented, unconfirmed): the 0x44-byte map cells (id word +0, kind +8,
// neighbour bits +0xc, direction +0xd, flags word +0xe, owner word +0x10, count word +0x20). Reimplementation of 0x5346d0 and 0x5367a0,
// which call each other. Agent C's unported 0x4de1f0 goes through the env.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the cell kinds call in the game, so that they can run outside it (the benches point these at recorders).
struct CellKindEnv {
    void(__fastcall *thing4de1f0)(u8 *thing);   // 0x4de1f0 (thiscall on a thing, no arguments)
};
// The cell kinds' calls go to env (nullptr: the game's).
void CellKind_SetEnv(const CellKindEnv *env);

// 0x5346d0 (thiscall (kind), pops 4; name invented, unconfirmed): 0 when the kind does not fit the cell (Kinds_Compatible). The cell's
// own kind again (not 0) counts up its +0x20 and gives 1. Else a price: for kind 1 the dword 0x81b6dc, for kind 3 0x81ea14, none while
// the byte 0x8186d4 is set; kind 3 on a type 1 cell first clears it (MapCell_Clear (0, 0) with 0x81d7a8 set to 1, then 0). Unless the
// game is free (0x785a2c), kinds 1 and 3 give 0 when the money (Source_Field0c of the world's table entry +0x1da726) less the price is
// negative (the sign of the wrapped difference). Then the kind is set; the map's block value of the cell (Map_SetBlockValue of
// [0x7cf83c] + 0x2d8) becomes the next count of 0x7cdb98, its position the argument's slot with the cell's id word in its low half (sic).
// Kind 1 on a type 3 cell: when the owner's cell ([0x7cf83c] + 0x294 + owner * 0x44) is of type 4, 9 or 0xa, its first object
// (Cell_FirstObject, out to the argument's slot) unless none gets the env's 0x4de1f0. The price is then debited (Money_Debit) for kind 1
// and for kind 3 on a cell not (now) of type 1, both only without 0x8186d4. Returns 1.
u32 __fastcall MapCell_SetKind(u8 *cell, void *, u32 kind);

// 0x5367a0 (thiscall (a, b), pops 8; name invented, unconfirmed): a cell cleared. A type 1 cell with flag 0x20 and no neighbours
// (MapCell_NeighbourCount) loses the flag; a cell left with flag 0x20 is left alone unless 0x81d7a8 is set. Then by its type:
// - 1 and 0xa (a path): the count +0x20 is -1 when a and b are both 0, else counted down; while it is not negative and 0x81d7a8 is 0,
//   nothing more. It becomes 0; then each of the 8 directions (1, 2, 4, ... 0x80: Byte_ShiftLog2 (d, 2) from 1 back to 1) among the
//   cell's neighbour bits is dropped from them and the next cell that way (Map_StepCellInDirection, unless none) loses the reverse
//   (Nibble_Scale) and gets its model (MapCell_ApplyPathModel). For a type 1 cell, a side direction (1, 4, 0x10, 0x40) and a next cell of
//   type 9 with an owner (+0x10), the owner's cell's first object (Cell_FirstObject, out to b's slot), unless none, gets the env's
//   0x4de1f0 when its type byte (+2) is 3. The direction lives in a's stack slot: its low byte, the rest a's (sic).
// - 3 and 9 (an object): the count is -1 when a and b are both 0 or the cell is a corner (two side neighbours, MapCell_SideNeighbourCount,
//   one of 1 / 0x10 with one of 4 / 0x40), else counted down. With 0x81d7a8 set it becomes 0 and a type 3 cell is scrapped; without, a
//   negative count becomes 0, a type 3 cell is scrapped and the 4 side directions (Byte_ShiftLog2 (d, 4)) are dropped as above (no
//   objects). Scrapped: when the owner's cell has a type (+8 not 0), its first object (Cell_FirstObject out to a's slot, read whether
//   found or not) is worth Object_ScrapValue times the dword 0x81ea14 over 100 (unsigned), credited (Money_Credit) to the world's money.
// - 4; 0x15 after its entry's (MapHandle_Entry28) dword +4 is cleared.
// - Others: nothing.
// Then the cell loses all neighbour bits, its direction, owner and flags, is set to kind 0 (MapCell_SetKind) and gets its model
// (MapCell_ApplyPathModel).
void __fastcall MapCell_Clear(u8 *cell, void *, u32 a, u32 b);

// 0x538fc0 (thiscall (kind), pops 4; name invented, unconfirmed): a track cell's (the 0x28-byte cells) kind set. The price: 0x81aebc for
// kind 0xb, 0x81b43c for 0x10, none with the byte 0x8186d4. A cell of another kind (not 0) gives kind == 0x19 unless it is of kind 0x19,
// 7 or 0, has flag 0x40 or the kind is 0; its own kind counts its +0x20 up and gives 1. Unless the game is free (0x785a2c), a kind other
// than 0x19, 0 and 7 gives 0 when the money (as MapCell_SetKind's) less the price is negative. The kind set and the block value as
// MapCell_SetKind's (the position's high half its own unset local, which the callee ignores). Kinds 0xb and 0x10 with 0x820ad0 set: the
// map cell of its id (MapHandle_Cell44) and the cells east, south-east and south of it (Map_StepCell by the pairs 0x7cdbc8, 0x7cdbb8,
// 0x7cdba8; any missing stops it), all four of kind 1 or 0, are cleared (MapCell_Clear (0, 0)) in that order. The price is debited for
// a kind other than 0 and 7 without 0x8186d4. Returns 1.
u32 __fastcall MapCell_SetTrackKind(u8 *cell, void *, u32 kind);

// 0x53bb20 (thiscall; name invented, unconfirmed): a track cell and those south, east and south-east of it (Map_StepCell28 by the pairs
// 0x7cdba8, 0x7cdbc8, 0x7cdbb8; missing ones skipped) emptied: all neighbour bits, direction and flags cleared, kind 0
// (MapCell_SetTrackKind), the owner 0 (MapCell_SetParent).
void __fastcall TrackCell_ClearBlock(u8 *cell);
}
