// Things taken off the map by area: the rectangle sweep 0x52bdd0, which demolishes (0x527ee0) the queue lines that lie wholly inside it,
// and that demolish, which takes an object off the map (its refund, its footprint's cells) and mends the paths about it.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the sweep and the demolish call in the game that is not ported yet (the benches point these at recorders).
struct DemolishEnv {
    void(__cdecl *demolish)(s32 x, s32 y, u32 flag);                         // 0x527ee0 (Path_CheckDeadEnds; the game's: the port)
    void(__fastcall *cell5367a0)(u8 *cell, void *, u32 a, u32 b);            // 0x5367a0 (agent B's, thiscall on a map cell, pops 8)
    void(__cdecl *thing530120)(u8 *thing);                                   // 0x530120 (agent B's; cdecl (thing))
    void(__cdecl *f52f200)(u32 a, u32 b);                                    // 0x52f200 (cdecl (a, b))
    void(__cdecl *f52f580)(u32 a, u32 b);                                    // 0x52f580 (cdecl (a, b))
    u32(__cdecl *step52fe50)(u8 *thing, u32 flag);                           // 0x52fe50 (cdecl (thing, flag); its answer tested)
    void(__fastcall *entry53b280)(u8 *entry, void *, u32 a, u32 b, u32 c);   // 0x53b280 (agent B's, thiscall on a 0x28-byte entry, pops 0xc)
    void(__cdecl *f545610)(u32 value);                                       // 0x545610 (agent B's; cdecl (the thing's dword +0x28))
};
// The sweep's and the demolish's calls go to env (nullptr: the game's).
void Demolish_SetEnv(const DemolishEnv *env);

// 0x52bdd0 (cdecl (x0, y0, x1, y1); name invented): over the rectangle's cells (signed corners in either order; the cell of (x, y) the
// world [0x7cf83c] + 0x294 + ((y << 7) + x + 1 as 16 bits) * 0x44), each of type 4, 9 or 10 (+8) whose line of cells of those types with
// its owner word +0x10, followed right, left, down and up from it while on the map (Map_InBounds), has no cell flagged 0x10
// (MapCell_HasFlags) and does not run more than one cell past the rectangle (each direction checked only while the ones before pass):
// demolished (0x527ee0 (x, y, 0)). Then each cell of type 1 in it gets 0x5367a0 (0, 0). Returns 1 (AL).
u8 __cdecl Map_DemolishRect(s32 x0, s32 y0, s32 x1, s32 y1);

// 0x527ee0 (cdecl (x, y, flag); name Path_CheckDeadEnds, low confidence): demolishes the object on the map cell (x, y) (Map_ObjectAtCell,
// which may move x and y to the object's cell; nothing for none or an empty slot). With the scrap value (Object_ScrapValue), the angle +0x10
// and the definition (Object_GetDef): a definition with +0x40 set gets env 0x530120 (thing), 0x52f200 (3, 0) and 0x81d7a8 = 1 for a while;
// a thing with +0x40 above 0 (signed) runs 0x52fe50 (thing, 1) until it answers 0 and is debited (Money_Debit of the money record,
// GameTable7cfb90_Get1da726_5195d0) [0x81ea14] * scrap / 100, else 0x820a8c = 0 (Fn_52fbd0). A definition kind +0x9c of 1 / 2 traces the
// track (TrackRun_TraceA, 0x52f580 (0xb / 0x10, 0), 0x52fe50 (thing, 0) until 0 with 0x81b46c 1; TrackRun_TraceB and the same) and is
// debited scrap * ([0x81aebc] / [0x81b43c] * 2) / 100; another kind but 0 a debit of 0. Without flag, 0x52f200 ([0x81af64] as read at the
// start, 0). Then the footprint (the definition's +0x18: columns, rows, from +8 rows of 0xa0 bytes, 8 bytes to a column, a dword not 0 for
// a cell taken), turned by the angle (0x5a: (x + row, y - column), 0xb4: (x - column, y - row), 0x10e: (x - row, y + column), else
// (x + column, y + row)), on the map: its type 4 cells counted; a type 9 cell's (Nibble_Scale of its direction) or a type 0xa cell's
// (its direction) next cell of type 1 with flag 0x20 loses it when at most one of the cells about it (from that direction round by
// Byte_ShiftLog2(d, 4), among its neighbour bits) is of type 9 or 0xa and its byte +0x26 lacks bit 3. A second pass clears the cells'
// flag 0x20, calls env 0x5367a0 (0, 0) on each and on its entry (MapHandle_Entry28) clears flag 0x10 with flag, else 0x53b280 (0, 0, 1).
// Without flag a thing with +0x28 set gets 0x545610 (+0x28); the thing is deleted (Thing_Delete). Unless sound is off (0x785a2c): sound 0x96,
// 0x97, 0x98 or 0x99 (type 4 cells under 4, 8, 16, or more) in [0x803a2c] at (__ftol(the last column count * [0x700cd8]), 0, __ftol(the
// rows * [0x700cd8])) (Sound_PlayAt; the column count is 0 for no columns and, sic, x as given for no rows).
void __cdecl Path_CheckDeadEnds(s32 x, s32 y, u32 flag);
}
