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
}
