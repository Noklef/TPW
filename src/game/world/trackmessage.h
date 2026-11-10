// A track cell's messages (0x5370e0; name invented, unconfirmed): the 0x28-byte map cells' (the track, hoarding and queue layer,
// [0x7cf83c] + (id + 0x6cde) * 0x28) counterpart of MapCell_HandleMessage (cellmessage.h). Map_ApplyLine (mapline.h) and the hoarding
// runs (cellrun.h) send them.
#pragma once

#include "../../../re/types.h"

// What the messages call in the game, so that they can run outside it (the benches point these at recorders; the ports by default).
struct TrackMessageEnv {
    u32(__fastcall *canBuild)(u8 *cell, void *, u32 kind, u32 alone, u32 direction);  // MapCell_CanBuildTrack (0x539760)
    void(__cdecl *addSquare)(u32 a, u32 b, u32 c, u8 d, u32 tag, u32 e, u32 f, u32 g, u8 h, u32 i, u32 j);  // DynFaces_AddSquare (0x53c8d0)
    void(__fastcall *remove)(u8 *cell, void *, s32 a, s32 b, u32 models);               // TrackCell_Remove (0x53b280)
    void(__fastcall *link)(u8 *cell, void *, u32 kind, s32 stepX, s32 stepY);          // MapLine_LinkCell (0x539220)
    void(__fastcall *stepDirection)(u8 *cell, void *, s32 dx, s32 dy);                 // MapCell_SetStepDirection (0x539670)
    void(__fastcall *pathModel)(u8 *cell);                                             // MapCell_ApplyPathModel (0x5365d0)
    void(__fastcall *model)(u8 *cell);                                                 // MapCell_ApplyModel (0x53afa0)
    u32(__fastcall *setKind)(u8 *cell, void *, u32 kind);                              // MapCell_SetTrackKind (0x538fc0)
    u8(__cdecl *updateA)(u8 *thing, u32 flash, u32 start, u32 bank5);                  // TrackRun_UpdateA (0x52c6f0)
    void(__cdecl *updateB)(u8 *thing, u32 flash, u32 start, u32 bank5);                // TrackRun_UpdateB (0x52ccb0)
    void(__cdecl *deadEnds)(s32 x, s32 y, u32 flag);                                   // Path_CheckDeadEnds (0x527ee0)
    void(__fastcall *clear)(u8 *cell, void *, u32 a, u32 b);                           // MapCell_Clear (0x5367a0)
    void(__cdecl *setOnly)(u32 mode, u32 record);                                      // GameMode_SetOnly (0x52f580)
};

// Points the messages at env (null: the game's).
void TrackMessage_SetEnv(const TrackMessageEnv *env);

extern "C" {

// 0x5370e0 (thiscall on a 0x28-byte cell; message, a, b, pops 12; name invented, unconfirmed): the message's low byte is the code.
// The cell's x and y are its id word - 1's low 7 bits and the rest (32-bit). A cell's height: for a type (+8) of 0xb, 0x10, 0x19, 7, 4,
// 9 or 0xa the ground at its x and y (Terrain_SurfaceHeight, no slot) rounded up to tens ((__ftol + 9) / 10, signed, times the float
// 0x700d94), else 0, then Terrain_RoundedCellHeight of its 0x44-byte cell (MapHandle_Cell44) with that height, x and y.
// - Walls (bit 0x100): the code and a / b kept when the code is 4, or 0xb or 0x10 with a 5 (else 0 and 1); the cell's height; the
//   direction byte 0x40 for a of -2, 4 for 2, then 1 for b of -2, 0x10 for 2 (the later wins; the rest of the pushed dword is the
//   stack's, sic); the answer MapCell_CanBuildTrack (code, 0, direction). Its 1: again with 1, Global816d48_Set (1), the square
//   DynFaces_AddSquare (x * 10, height - the float 0x700d9c, y * 10, 0, 1, 1, 0, 1, 0, 0, 0) (the coordinates floats from the 32-bit
//   products) and for each side (pairs 0x7cdba0, 0x7cdbc8, 0x7cdba8, 0x7cdbd0; directions 1, 4, 0x10, 0x40) the cell Map_StepCell28
//   reaches with a scale of 2, when there is one and its height is ordered and not the cell's: a square (.., side direction, 1, 0, 0, 1,
//   0, 0, 0) at each __ftol(its height) + 10k below __ftol(the cell's), less 0x700d9c. Any other answer: the square with the answer (its
//   0 the kept a) and the kept b, the sides' squares with tag 0 and i = the answer was 0xd. Answers 1.
// - 0x32: TrackCell_Remove (a, b, 1). 0x80: MapLine_LinkCell (0x816d64, a, b). 0x82: MapCell_SetStepDirection (a, b). 1.
// - 0x81 (model): 1 at once when 0x81b4cc's byte is set and the type is 0xb, 0xc, 0x10 or 0x11 or its 0x44-byte cell's 0x15. A type
//   0x19 (hoarding) cell: its 0x44-byte cell's flag 0x40 cleared or set (MapCell_Flags | 0x40) by the cell's direction and neighbour
//   bits, MapCell_ApplyPathModel on it, then the same for the 0x44-byte cells one step by the pairs 0x7cdbc8, 0x7cdba8 and 0x7cdbb8
//   (Map_StepCell; none skipped; set from the cell's own flags, sic): see hoarding_flags. Others: a corner (two side neighbours at a
//   right angle) or a junction (more than two side neighbours, signed, with any neighbour bit) loses the low 4 bits of +0xe. Then
//   0x816d64 = 0, MapCell_ApplyModel unless of type 0xd or 0x12, and with 0x820ad0 and 0x820ae8 set 0x820ae8 cleared and 0; else 1.
// - 0x83 (owner): as 0x81's first test; with 0x8186c4 set +0x10 = 0x818698's word and the cells Map_StepCell28 reaches by the pairs
//   0x7cdba8, 0x7cdbc8, 0x7cdbb8 (scale 1) get the cell's id as their +0x10 and MapCell_SetTrackKind (0xc). 1.
// - 0x85 (end flag): 1 for a corner or a junction. A hoarding: +0xe = 1 when its 0x44-byte cell or one of the three it steps to (pairs
//   0x7cdba8, 0x7cdbc8, 0x7cdbb8) is of type 2, 7 or 0x1e. Others: +0xe = 1 for those of type 1 or 3; four of type 0 clear +0xe's low
//   4 bits. 1.
// - 0x86 (run): a type 0xb, 0xd, 0x10 or 0x12 cell with 0x81b4cc's byte clear and an owner word (+0x10): the owner's world cell's
//   (0x44 each from [0x7cf83c] + 0x294) first object (Cell_FirstObject, out to b's slot), when there is one, has its thing (0x7cfb90,
//   20 bytes each) redrawn: TrackRun_UpdateA (thing, 1, 0, 0), its 0 TrackRun_UpdateB (thing, 0, 0, 0). 1.
// - 0x87 (dead ends): with 0x818c10 > 0x818c14 (signed): 0x81ede4 / 0x81ede8 kept, the mode (0x81ae2c); unless of type 0xb, 0xd, 0x10,
//   0x12 or 0x19 Path_CheckDeadEnds at (x, y), (x + 1, y), (x, y + 1), (x + 1, y + 1) (flag 0) and with 0x820ad0 MapCell_Clear (0, 0) of
//   the 0x44-byte cell and the three it steps to (unchecked, sic); the two put back and GameMode_SetOnly (the mode, 0). 1.
// - Any other (0x84 too): 0x820ae8 = 1 for type 0x19, or 0xb, 0xd, 0x10, 0x12 with 0x820ad0 set, else 0; MapCell_SetTrackKind of the
//   whole message: its 0 answers the message was 0x19, else 0x816d64 = the message and 1.
u32 __fastcall MapCell_HandleTrackMessage(u8 *cell, void *, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const float, gTrackmessage_700d94, 0x00700d94, 0x4);
GAME_DATA(const float, gTrackmessage_700d9c, 0x00700d9c, 0x4);
