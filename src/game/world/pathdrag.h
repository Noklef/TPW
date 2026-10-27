// A path drag's held cell: the map id's column and row at 0x763a00 / 0x763a04 (-1: none), the held flag 0x818c54 and the value taken
// from the cell's entry +4 at 0x818c58. Reimplementation of 0x52c150 and 0x52c1c0 (cdecl). 0x5367a0 and 0x536100 are agent B's and not
// yet ported: the env calls them.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the path drag calls in the game, so that it can run outside it (the benches point these at recorders).
struct PathDragEnv {
    void(__fastcall *cell5367a0)(u8 *cell, void *, u32 a, u32 b);              // 0x5367a0 (thiscall on a map cell, pops 8)
    void(__cdecl *step536100)(u32 code, s32 x, s32 y, s32 *outX, s32 *outY);   // 0x536100
};
// The path drag's calls go to env (nullptr: the game's).
void PathDrag_SetEnv(const PathDragEnv *env);
}
