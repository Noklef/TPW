// The track circuit walked (0x52a970, cdecl (check); name invented, unconfirmed; called from 0x524d0a, 0x52693e, 0x526bd0 and 0x526f02,
// the result kept at the thing's +0x2c). From the station's cell 0x7639e0 / 0x7639e4 moved four cells back by the angle at 0x7639ac
// (0: x - 4, 0x5a: y + 4, 0xb4: x + 4, 0x10e: y - 4, another: none), in the direction byte at 0x818c50, two cells a step until back at
// that cell or failed. The start cell (taken for the cell its +0x10 word names when of type 0xc or 0x11) not of type 0xb, 0xd, 0x10, 0x12
// or 0x19 returns 0 at once. The ride is the +0x28 of the first object (Cell_FirstObject) of the 0x44-byte cell (MapHandle_Cell44) of
// the cell the start cell's +0x10 names (the thing's slot read without a check); with check's low byte 0 it is stripped first
// (TrakRide_Strip) and the sections are added (TrakRide_AddSection, at the cell's x and y times 3072 through __ftol: their low dwords).
//
// Each cell of those types: a corner (MapCell_CornerNeighbours) adds piece 5, 8, 6 or 7 for the neighbour bits 5, 0x14, 0x41, 0x50
// (others none; extra arguments 0); a junction piece 0xb. Either resets the upgrade count and the mesh. A straight: piece 0xa / 0xd for
// north-south, 9 / 0xc for east-west (the second with flag 0x10, MapCell_HasFlags), 0x10000 or'd in for the first two cells walked,
// with the upgrade count, the mesh (+0x20) and the definition's +0x10 (Object_GetDef) of the object at the cell
// (Map_ObjectOrEntranceAt, when flag 0x10; else the count and mesh reset) as its last three arguments. Then a cell with flag 0x10 is an
// upgrade: its count goes up (wrapping to 0 clears its sides); the object at the cell its first word names, when there is one, has
// sides: 0x10 when the cell two on in the cell's direction (MapCell_Direction; Map_StepCell28InDirection) is a track cell
// (MapCell_IsTypeBOr10) with a thing (+4) or of type 0xd or 0x12, 1 likewise behind (Nibble_Scale); at the second upgrade the parts
// keyed 1 and 0x10 (Model_FindPartByKey8, mask 1) of its thing (+0x20, entry of 0x7a4610) get flag 0x10 cleared (no side) or
// 0x80000010 set (a side's part: 1 for 0x10, 0x10 for 1). A cell without flag 0x10, with no thing and of type 0xb or 0x10 gets its
// model (MapCell_ApplyModel). A corner then turns the direction (Nibble_Scale of it when among its neighbour bits, else the direction
// itself, xor the neighbour bits) and a cell whose direction differs gets it (MapCell_SetDirection) and its model.
//
// The walk fails (0) on a cell of another type (the direction 0), a direction 0, fewer than two neighbour bits, or with three: when the
// cell two on is of the types above and has no neighbour bits while the direction has bit 1, or when that cell's type is 0 (the cell
// not checked for null, sic). Each cell gets its index in the walk (MapCell_SetWord24). The walk moves two cells (1: y - 2, 4: x + 2,
// 0x10: y + 2, 0x40: x - 2; another direction stays, sic). Returns 1 when back at the start.
#pragma once

#include "../../../re/types.h"

extern "C" {
u32 __cdecl TrackCircuit_Walk(u32 check);

// What the walk calls in agent B's code, so that it can run outside the game (the benches point it at recorders).
struct TrackCircuitEnv {
    void(__cdecl *strip)(u32 ride);                                                   // TrakRide_Strip 0x545450
    s32(__cdecl *addSection)(u32 ride, s32 x, s32 y, u32 piece, u32 a, u32 b, u32 c);  // TrakRide_AddSection 0x54b2f0
    void(__fastcall *applyModel)(u8 *cell);                                           // MapCell_ApplyModel 0x53afa0
};
// The walk's calls go to env (nullptr: the game's).
void TrackCircuit_SetEnv(const TrackCircuitEnv *env);
}
