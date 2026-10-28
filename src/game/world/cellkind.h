// A map cell's kind (+8) changed and a cell cleared (names invented, unconfirmed): the 0x44-byte map cells (id word +0, kind +8,
// neighbour bits +0xc, direction +0xd, flags word +0xe, owner word +0x10, count word +0x20). Reimplementation of 0x5346d0 and 0x5367a0,
// which call each other, and of the track cells' 0x538fc0, 0x53bb20 and 0x53b280. Agent C's unported 0x4de1f0 and agent A's 0x527ee0
// (Path_CheckDeadEnds, not ported here yet) go through the env.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the cell kinds call in the game, so that they can run outside it (the benches point these at recorders).
struct CellKindEnv {
    void(__fastcall *thing4de1f0)(u8 *thing);                // 0x4de1f0 (thiscall on a thing, no arguments)
    void(__cdecl *deadEnds527ee0)(s32 x, s32 y, u32 flag);  // Path_CheckDeadEnds (0x527ee0, agent A's)
};
// The cell kinds' calls go to env (nullptr: the game's 0x4de1f0, the port of Path_CheckDeadEnds).
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

// 0x53af00 (fastcall; name invented, unconfirmed): a track cell reset: its kind 0 on park land (Terrain_IsParkLand of its id's x, y),
// else 7 (MapCell_SetTrackKind); all its neighbours dropped (MapCell_RemoveNeighbours 0xff) without, then with 0x81b4cc set (the
// hoarding neighbours; the byte cleared after); direction 0 (MapCell_SetDirection), flags (+0xe) 0, owner 0 (MapCell_SetParent); +4,
// the count +0x20 and +0x14-+0x1f cleared, +0x24 0xffff.
void __fastcall TrackCell_Reset(u8 *cell);

// 0x536490 (thiscall (models), pops 4): a map cell reset from its attribute byte (+0x26): neighbours (MapCell_RemoveNeighbours 0xff),
// direction and owner cleared, flags (+0xe) and the model (+0x14-+0x1f) 0. On park land (Terrain_IsParkLand of its id's x, y) the kind
// 1 with bit 8, joined as a path (MapCell_LinkPath (1, 0, 0, models)) and the flags 0x20, else the kind 0; off it the kind 2 (bit 2),
// 0x1e (bit 0x80) or 7. Without flag 0x20 the flag 0x40, and the path model (MapCell_ApplyPathModel) when models' low byte is set. Bit
// 0x10 sets flag 0x80, bit 4 flag 0x400; +4 and the count +0x20 cleared.
void __fastcall MapCell_Reset(u8 *cell, void *, u32 models);

// 0x53b280 (thiscall (a, b, models), pops 0xc; name invented, unconfirmed): a line cell (the world's 0x28-byte cells, [0x7cf83c] +
// 0x1102b0 + id * 0x28) removed. The price: 0x81aebc for kind 0xb, 0x81b43c for 0x10. A cell of kind 0xc or 0x11 removes its owner's
// cell (+0x10) instead (a, b, models 1). Else the line's start and end ids (0x7639e0 / 0x7639e4, 0x7639e8 / 0x7639ec, 0 when out of the
// map), and for the line kinds 0xb, 0xd, 0x10, 0x12, 0x19 (jump tables 0x53babc / 0x53baa4) the count (+0x20): -1 for a and b 0 off the
// start; else counted down, then: a and b -1 drop the sides across the cell's direction (1 / 0x10: 4 and 0x40; 4 / 0x40: 1 and 0x10);
// a cell with two side directions (MapCell_SideDirectionCount) loses the direction bits a / b point away from (a -2: 4, 2: 0x40; b -2:
// 0x10, 2: 1) and the side pairs of a / b +-2; any other cell but the start's and the end's (those not even a model) loses the side a / b
// point at, and as a corner (two side neighbours, one of 1 / 0x10 with one of 4 / 0x40) left with one neighbour faces its reverse
// (Nibble_Scale); the cell gets its model. It is emptied when its count is negative off the start and end, negative with 0x81b46c set, or
// 0x81d7a8 is set: the count 0; unless of kind 0x19 or with 0x785a2c set, the first object (Cell_FirstObject, out to a's slot, read
// whether found or not) of its owner's world cell's 0x44-byte cell (MapHandle_Cell44) credits Object_ScrapValue times the price over
// 100 (unsigned) to the world's money; each of its 8 neighbour directions (Byte_ShiftLog2 (d, 4) from 1, in a's slot's low byte) is
// dropped from it and the reverse from the cell two steps that way (Map_StepCell28InDirection), which gets its model and, left with one
// neighbour, faces its reverse unless it faces it or that already; flag 0x10 (+0xe) calls the env's 0x527ee0 (x, y, 1) of its id (put in
// a's slot); its direction, flags and kind (MapCell_SetTrackKind 0) cleared, and unless it is then of kind 0x19 its owner (0) and +0x24
// (-1); its model; the cells south, east and south-east (Map_StepCell28 by 0x7cdba8, 0x7cdbc8, 0x7cdbb8) it owns emptied (neighbours,
// direction, flags, kind 0, count 0, the owner unless of kind 0x19) with a model when the byte models is set. Last, for the start's cell
// of a line kind without 0x81b46c: each direction not among its bits whose cell two steps out joins the line (of kind 0xd / 0x12, or 0xc /
// 0x11 with its owner's world cell of those, and of the cell's owner) is faced and joined (MapCell_AddNeighbours, a model), and one whose
// cell is not of a line kind is dropped (a model); left with one neighbour it loses its direction bits and faces those that join (in
// models' slot's low byte). A cell of a kind not 0 without a ride (+4) gets its model.
void __fastcall TrackCell_Remove(u8 *cell, void *, s32 a, s32 b, u32 models);
}
