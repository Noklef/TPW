// The object being placed made (0x529e10): the definition and cell the placing mode keeps (0x818c38 / 0x818c3c), a track ride for
// definitions that have one, the object constructed on the heap.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the placement calls in the game that is not ported yet (the benches point it at a recorder).
struct PlaceObjectEnv {
    u8 *(__fastcall *construct)(u8 *block, void *, u32 cell, u32 id, s32 angle, u32 ride);   // 0x4db090 (agent C's object constructor, pops 0x10)
};
// The placement's calls go to env (nullptr: the game's).
void PlaceObject_SetEnv(const PlaceObjectEnv *env);

// 0x529e10 (cdecl (angle); name invented, unconfirmed): the object being placed made. The definition of the id at 0x818c38
// (ObjectDefs_Find) with a ride kind (+0xa0) gets a track ride (TrakRide_Create (kind, x, y, code, 0, id)): from the cell at
// 0x818c3c (its word less 1: column, row) offset by the definition's pair for the angle (0: +0xa4 / +0xa8, code 0; 0x5a: +0xac / +0xb0,
// code 5; 0xb4: +0xb4 / +0xb8, code 6; 0x10e: +0xbc / +0xc0, code 1; another: none, the code the angle) and then by Position_StepA's
// step (0: (4, 1), 0x5a: (1, -5), 0xb4: (-5, -2), 0x10e: (-2, 4)), each times [0x700cd0] times [0x700cd8] truncated (__ftol). The
// object's cell is 0x818c3c's word plus the definition's pair +0x4b0 turned by the angle (IntPair_Rotated: column, row << 7, 16 bits).
// A block of 0x450 bytes (Mem_Alloc; none: 0) is constructed by 0x4db090 (cell, id, angle, the ride's handle or 0); a made object
// gets as its word +0x30 (Fn_4e0de0) the handle of the thing at [0x8186ac] (Thing_GetHandle) when the flag 0x8187f8 is set and the
// definition's +0x288 is not 2, else 0, and becomes 0x7c2654 (Wrap_Global7c2654_Set_4815b0). Returns the object (0 when none).
// (The original passes the cell and the id with stack bytes in their high halves; 0x4db090 reads their low words only.)
u8 *__cdecl PlaceObject_Create(s32 angle);

// What the footprint calls (the benches point these at recorders): the game's unported ones by address, the rest the ports.
struct PlaceFootprintEnv {
    u32(__fastcall *test532fc0)(u8 *cell, void *, u32 action, u32 b, u32 c);             // 0x532fc0 (agent B's, thiscall on a 0x44-byte cell, pops 0xc)
    void(__fastcall *clear)(u8 *cell, void *, u32 a, u32 b);                            // MapCell_Clear 0x5367a0
    void(__fastcall *entry536780)(u8 *entry);                                           // 0x536780 (agent B's, thiscall on a 0x28-byte entry)
    void(__fastcall *thing4de1f0)(u8 *thing);                                           // 0x4de1f0 (agent C's, thiscall on the thing [0x8186ac])
    void(__fastcall *linkPath)(u8 *cell, void *, u32 mode, s32 dx, s32 dy, u32 models);  // MapCell_LinkPath 0x5348d0
    u8 *(__cdecl *create)(s32 angle);                                                   // PlaceObject_Create 0x529e10
    void(__cdecl *focusUpdate)(u32 record);                                             // FocusCell_Update 0x5276d0
    u32(__cdecl *playAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);      // Sound_PlayAt 0x51bfc0
};
// The footprint's calls go to env (nullptr: the defaults).
void PlaceFootprint_SetEnv(const PlaceFootprintEnv *env);

// 0x528a70 (cdecl (x, y, unused, angle, place, check); name invented, unconfirmed): the footprint of the object being placed (the
// definition of the id at [0x8186e0], kept at 0x818c38 / 0x818c40; its shape +0x18 at 0x818c18: columns, rows, from +8 rows of 0xa0
// bytes, 8 bytes to a column: a type dword and a direction byte +4) laid on (place) or tested against (check, else a soft test) the
// map at (x, y): (x, y) less half the columns / rows unless [0x81af7c], each cell turned by the angle (0x5a, 0xb4, 0x10e, other) and
// skipped off the map. 0x7639e0 / e4 and 0x818c20 - 0x818c2c start at -1, 0x818c44 / 48 / 4c the corner and the angle. Placing first
// marks each typed cell (0x532fc0 0x87), counts the type 4 ones, keeps the first cell (0x818c28 / 2c / 3c), makes the object
// (PlaceObject_Create) and plays 0x8e / 0x8d / 0x8f / 0x90 (under 4, 8, 16 type 4 cells, or more) at the corner times [0x700cd8]
// unless sound is off. Then per cell: placing re-types it (0x87, MapCell_Clear, its type (0xb / 0x10 / 0x17 as 4), 0x81, the object's
// cell its parent, 0x536780 and flags 0xf off its entry); by type: 1 a path (flag 0x20, linked with the path cells about it), 9 and
// 0xa the ends (their direction the angle's turned by the byte, the cell behind / ahead kept; placing points them and links the path
// cell they face; 9's at 0x818c20 / 24 / 30), 0xb / 0x10 flag 0x10 on the cell and its entry, 0x17 the cell it faces; the tests ask
// 0x532fc0 (0x204 / 0x104 / 0x20b / 0x110 ...), a failed hard one returning 0, a failed soft one setting [0x816d50]. With an end the
// cells behind 9 and ahead of 0xa (off the map: 0) are joined (by Global81b0cc_IfNoFlag: to the path or as the queue's ends; 0x4de1f0
// on [0x8186ac]); for a track kind 1 or 2 (Global81ecac_IfNoFlag) two TrackSquare_Checks at the definition's offsets for the angle
// (+0xa4 / +0xa8, 0x10e +0xac / +0xb0, 0xb4 +0xb4 / +0xb8, 0x5a +0xbc / +0xc0). Returns 0x818c20 (the type 9 cell's point), else 0.
// (Stack bytes the original leaves unset reach only callees that read their low byte. The direction given to TrackSquare_Check is the
// last cell's (1, 0x40, 0x10, 4 by the angle); with no columns it is the placing pass's last row coordinate, sic, and with no rows, or
// with no columns and no placing, unset in the original: the port passes 0 there.)
s32 *__cdecl PlaceObject_Footprint(s32 x, s32 y, u32 unused, s32 angle, u32 place, u32 check);
}
