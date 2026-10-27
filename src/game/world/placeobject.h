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
}
