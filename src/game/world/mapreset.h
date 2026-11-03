// The map's cells reset at a new park. Reimplementation of 0x52f8d0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct MapResetEnv {
    void(__fastcall *mapReset)(u8 *cell, void *, u32 models);   // MapCell_Reset
    void(__fastcall *trackReset)(u8 *cell);                     // TrackCell_Reset
};

void MapReset_SetEnv(const MapResetEnv *env);

// 0x52f8d0 (cdecl (columns, rows); called from 0x41503c with 0x80, 0x80): for each row and column (signed, none at or below 0) the cell id
// (u16)((row << 7) + column + 1): on the map [0x7cf83c] the 0x44-byte cell at +0x294 reset (models 1) and the 0x28-byte track cell at id + 0x6cde.
void __cdecl Map_ResetCells(s32 columns, s32 rows);
