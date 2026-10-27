// Things taken off the map by area: the rectangle sweep 0x52bdd0, which demolishes (0x527ee0) the queue lines that lie wholly inside it.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the sweep calls in the game that is not ported yet (the benches point these at recorders).
struct DemolishEnv {
    void(__cdecl *demolish)(s32 x, s32 y, u32 flag);                // 0x527ee0 (Path_CheckDeadEnds; cdecl (x, y, flag))
    void(__fastcall *cell5367a0)(u8 *cell, void *, u32 a, u32 b);   // 0x5367a0 (agent B's, thiscall on a map cell, pops 8)
};
// The sweep's calls go to env (nullptr: the game's).
void Demolish_SetEnv(const DemolishEnv *env);

// 0x52bdd0 (cdecl (x0, y0, x1, y1); name invented): over the rectangle's cells (signed corners in either order; the cell of (x, y) the
// world [0x7cf83c] + 0x294 + ((y << 7) + x + 1 as 16 bits) * 0x44), each of type 4, 9 or 10 (+8) whose line of cells of those types with
// its owner word +0x10, followed right, left, down and up from it while on the map (Map_InBounds), has no cell flagged 0x10
// (MapCell_HasFlags) and does not run more than one cell past the rectangle (each direction checked only while the ones before pass):
// demolished (0x527ee0 (x, y, 0)). Then each cell of type 1 in it gets 0x5367a0 (0, 0). Returns 1 (AL).
u8 __cdecl Map_DemolishRect(s32 x0, s32 y0, s32 x1, s32 y1);
}
