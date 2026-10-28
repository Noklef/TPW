// A 0x44-byte map cell's messages (name invented, unconfirmed): reimplementation of 0x532fc0, the handler the map's tools send a cell
// (id word +0, type +8). Its calls into agent A's unported 0x527ee0 (Path_CheckDeadEnds) and 0x52f580 (the mouse mode set again) go
// through an env.
#pragma once

#include "../../../re/types.h"

// What the cell messages call in the game's unported code, so that they can run outside it (the benches point these at recorders).
struct CellMessageEnv {
    void(__cdecl *deadEnds)(s32 x, s32 y, u32 flag);   // 0x527ee0 (Path_CheckDeadEnds, agent A's)
    void(__cdecl *setMode)(u32 mode, u32 record);      // 0x52f580 (agent A's)
};
// The cell messages' calls go to env (nullptr: the game's).
void CellMessage_SetEnv(const CellMessageEnv *env);

extern "C" {

// 0x532fc0 (thiscall (cell, message, a, b), pops 12; name invented, unconfirmed). Returns 1 unless said otherwise.
// - Flag 0x200: Global81ae2c_Get_52f840 (its answer dropped); whether MapCell_CanBuild (the low byte, 0) is not 1.
// - Flag 0x100 (a cell's walls drawn): x, y from the id ((id - 1) & 0x7f, (id - 1) >> 7, 32-bit); the mode 0x81ae2c (Global81ae2c_Get_52f840)
//   not 4 kept; MapCell_IsTypeCOr11 of its entry (MapHandle_Entry28) asked (the answer dropped). (a, b) kept for the low byte 4, 0x18, or 0xb
//   / 0x10 with a 5; else (0, 1). A cell's height: 0, or with its entry's type (+8) 0xb, 0xc, 0x10, 0x11 or 0x19 the rounded ground
//   (((__ftol(Terrain_SurfaceHeight(x, y, null)) + 9) / 10, signed) times the float 0x700d54, fild / fmul); a cell type of 7, 0x1e, 4, 9 or
//   0xa gives the rounded ground too. With 0x81b604 set the build answer is 1, else MapCell_CanBuild (the low byte, 0). For 1: MapCell_CanBuild
//   (the low byte, 1), Global816d48_Set(1) and a square face (DynFaces_AddSquare (x * 10, height - the float 0x700d5c, y * 10 as floats, 0, 1,
//   1, 1.0, 0, 0, 0, 0)); then for the pairs 0x7cdba0, 0x7cdbc8, 0x7cdba8, 0x7cdbd0 (sides 1, 4, 0x10, 0x40) the cell Map_StepCell (pair,
//   1) gives, if any, with a height other than the cell's (x87 compare), a square for each ten from __ftol of its height up to below
//   __ftol of the cell's (the height that ten - 0x700d5c; side, 1, 0, 1.0, 0, 0, 0, 0). Otherwise the answer (0: the kept a) is the
//   square's tag (the kept b after it), and the sides' squares have the tag 2 for an answer of 2, else 0, then 0, 1.0, 0, 0, the mode not
//   4 and 0, each after 0x816d58 and 0x816d50 are cleared.
// - Otherwise by the low byte: 0x32 MapCell_Clear (a, b); then the entry (its parent [0x7cf83c] + (its word +0x10 + 0x6cde) * 0x28 when
//   MapCell_IsTypeCOr11) of a type MapCell_IsTypeBDOr10Or12Or19 with end flags (MapCell_HasLowFlags), when neither its 0x44-byte cell
//   (MapHandle_Cell44) nor the cells Map_StepCell (0x7cdbc8, 0x7cdba8, 0x7cdbb8; 1) gives of it have type 1 or 3, by its direction
//   (MapCell_Direction) 1 / 0x10 or 4 / 0x40, with end flags still, has its ends reset (TrackCell_ResetEnds of the pairs 0x7cdba8 and
//   0x7cdba0, or 0x7cdbd0 and 0x7cdbc8). 0x80: MapCell_LinkPath ([0x816d64], a, b, 1). 0x81: MapCell_ApplyPathModel, 0x816d64 cleared;
//   with 0x820ab8 and 0x820abc both set, 0x820abc cleared and 0 returned. 0x82: with no direction, a -1 / 1 sets the direction 4 / 0x40,
//   then b -1 / 1 sets 0x10 / 1. 0x83: the word +0x10 = [0x818698]. 0x84: the word +0x40 = [0x8186bc], which counts up. 0x85: the entry (as
//   0x32's) of a type MapCell_IsTypeBDOr10Or12Or19 other than 0x19, at no corner nor junction (MapCell_CornerNeighbours /
//   JunctionNeighbours) and without end flags, gets end flag 1 (MapCell_ClearFlags 0xf, MapCell_SetFlags 1). 0x86: for the entry (as 0x32's)
//   of a type MapCell_IsTypeBDOr10Or12Or19 not MapCell_IsType19 with a parent (its word +0x10), the first object (Cell_FirstObject of the
//   world cell [0x7cf83c] + 0x294 + parent * 0x44), if any, has its thing (the dword at 0x7cfb90 + handle * 20) updated: TrackRun_UpdateA
//   (thing, 1, 1, v), and when that gives 0 TrackRun_UpdateB (thing, 0, 1, v); v is a's slot after the handle's word and then the byte
//   (thing's +0x2c not 0) went into it (sic). 0x87: when 0x818c10 > 0x818c14 (signed), a type 1 cell, unless the mode (Global81ae2c_Get_52f840)
//   is 3 or 1, is cleared (MapCell_Clear (0, 0)), else the env's deadEnds (x, y, 0); 0x81ede4 / 0x81ede8 put back as they were and the
//   env's setMode (the mode, 0). Others: 0x820abc = 1 when 0x820ab8 is set, the message is not 0x15 and the type is 3 or 1, or the message
//   is 0x15 and the cell has flag 0x100 (+0xe) or a type of 4, 9 or 0xa (else 0); MapCell_SetKind (the message); 0 for its 0, else
//   0x816d64 = the message.
u32 __fastcall MapCell_HandleMessage(u8 *cell, void *, u32 message, u32 a, u32 b);
}
