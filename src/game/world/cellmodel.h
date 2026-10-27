// A map cell's model picked from its neighbours (0x53a540; names unconfirmed): the answer is the model (its group in the high word, index in the
// low; the caller keeps them at +0x14 / +0x18 of the cell) and *angle (kept at +0x1c) its rotation in degrees. *angle is first 0; a cell whose
// type's magnitude (+8, signed) is 4, 5, 7, 9, 0xa, 0xc or 0x11 gets 8 straight away. MapHandle_Cell44 is asked for the cell's 0x44-byte cell and
// the answer dropped (sic).
//
// The model comes from a table of 12-byte rows (model, angle, neighbour bits byte at +8): 0x762f30 for a type of magnitude 3 or 0xb (its row
// count at 0x700c18), 0x762ff0 for 0x10 or 0x19 (0x700c1c), else 0x763138 (0x700c24). For the first two tables a straight cell (N-S or E-W
// neighbours, MapCell_StraightNeighbours) facing 0x40 or 0x10 (MapCell_Direction) turns by 180. For 0x19 a cell at either of the two cells kept
// at 0x7639f0 / 0x7639f4 and 0x7639f8 / 0x7639fc (its id - 1's column and row) has its neighbour bits or'ed with their reverse (Nibble_Scale)
// first. The first row with exactly the cell's neighbour bits gives the model and *angle = its angle plus the turn; failing that (or when that
// row's model is -1, sic) the first row whose bits are all among the cell's; failing that the model is -1.
//
// Then, by the type itself (not its magnitude):
// - a cell neither at a corner nor at a junction (MapCell_CornerNeighbours / JunctionNeighbours) adds 9, 0xc and 0xf for its flags (+0xe) 1,
//   2 and 8; flag 4 instead keeps the high word and sets the low to 0x12;
// - 0x10 or 0xb at a corner facing along it (0x40 with S and W, 0x10 with N... see the code) adds 0x10;
// - 0x10 at a junction facing 0x14 sets *angle 0x5a; 0x41 adds 0x10; 0x50 adds 0x10 and sets *angle 0x10e;
// - 0x19 at a corner adds 0x10 for some facings (else not), then becomes 0x1b (0x1a) when the cell's flags have any of the low 4; a straight
//   0x19 cell without them adds a variant every so often: the counter at 0x820aec picks 0 (under 0x12), 0x14 (under 0x18), 0x17 (under 0x1e)
//   or else the angle pointer's value (sic); it is added when it differs from the last (0x820af0), which it then becomes, and the counter is
//   set to World_Rand % 30. A 0x19 cell's answer then keeps the low word and has group 5.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x53a540 (thiscall on the cell; angle out, pops 4).
u32 __fastcall MapCell_PickModel(const u8 *cell, void *, s32 *angle);

// 0x535dd0 (thiscall on the cell; angle out, pops 4; name invented, unconfirmed): the same pick for a path's cell. *angle first 0; a type
// of magnitude 2, 4, 7, 9, 0xa, 0x15 or 0x1e gets 8 straight away. Magnitude 3 searches 0x7630b0 (its row count at 0x700c20) with a turn
// of 180 for a straight cell (MapCell_StraightNeighbours, inlined) not facing 0x40 or 0x10; any other 0x763138 (0x700c24) with none. Then a
// type 3 corner (MapCell_CornerNeighbours, inlined) facing along it (0x40 with 0x50, 0x10 with 0x14, 1 with 0x41, 4 with 5) adds 1. With
// the toggle byte 0x820ac0 set, a model of group 1 index 2 adds 0x11 and then one of index 0xa adds 0xa; the toggle then becomes rand() % 2
// (the CRT's). A model of group 2 adds 3 for each side (1, 4, 0x10, 0x40: Byte_ShiftLog2 (side, 4) round to 1) among the cell's neighbour
// bits whose next cell (Map_StepCellInDirection) is of type 1 with the side's reverse (Nibble_Scale) among its own bits, while the cell's
// own entry (MapHandle_Entry28; for a type 0xc / 0x11 one, MapCell_IsTypeCOr11, the parent [0x7cf83c] + (its word +0x10 + 0x6cde) * 0x28)
// has none of its low flags (MapCell_HasLowFlags).
u32 __fastcall MapCell_PickPathModel(const u8 *cell, void *, s32 *angle);

// 0x5365d0 (thiscall; name invented, unconfirmed): a path cell's model applied. Its kind (+8) positive but 5 picks the model
// (MapCell_PickPathModel), else 0x37 at angle 0; the model's high half (signed), low half and the angle go to 0x820aa8 / 0x820aac /
// 0x820ab0 and the cell's +0x14 / +0x18 / +0x1c.
// The cell's position: its id (the word +0) - 1, x its low 7 bits, y the rest. A model of group 2 (+0x14) replaces the cell's queue ride (+4:
// QueuePiece_Remove, then QueuePiece_Place (x, y, +0x14, +0x18, +0x1c, no claim)) and paints the erasing tile (0x820aa8 = {0, 8, 0}); any
// other removes the ride and paints its model (Terrain_PaintAt (x, y, the cell + 0x14)).
void __fastcall MapCell_ApplyPathModel(u8 *cell);
}
