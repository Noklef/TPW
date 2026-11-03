// The sweeps of the map's cells. Reimplementation of 0x4d8330 and 0x4d83a0; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; 0x536780 and 0x5366e0 unported, agent B's, by address; the bench's recorders).
struct MapSweepEnv {
    void(__fastcall *reset)(u8 *cell);        // 0x536780
    void(__fastcall *place)(u8 *cell);        // 0x5366e0
    void(__fastcall *placeRide)(u8 *cell);    // MapCell_PlaceRide
};
void MapSweep_SetEnv(const MapSweepEnv *env);

extern "C" {

// Both thiscall, on the map: for each of the 0x4000 cells w = y * 0x80 + x + 1 (y, then x, from 0) the cell of the 0x44-byte table at map + (w - 1) * 0x44 and the cell of the 0x28-byte table at map + 0x10ffd8 + w * 0x28.
// 0x4d8330: 0x536780 on each. 0x4d83a0: 0x5366e0 on the first, MapCell_PlaceRide on the second.
void __fastcall MapSweep_Reset(u8 *map);
void __fastcall MapSweep_Place(u8 *map);
}
