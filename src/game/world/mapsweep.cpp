// The sweeps of the map's cells: see mapsweep.h.
#include "mapsweep.h"

#include <cstdint>

#include "cellmodel.h"
#include "queuepieces.h"
#include "../core/addrcalls.h"

namespace {

using CellFn = void(__fastcall *)(u8 *);

const MapSweepEnv kGameEnv = {GameAddress<CellFn>(0x00536780), reinterpret_cast<CellFn>(CellPaint_Apply), reinterpret_cast<CellFn>(MapCell_PlaceRide)};
const MapSweepEnv *g_env = &kGameEnv;

void sweep(u8 *map, CellFn first, CellFn second) {
    for (u32 y = 0; y < 0x80; ++y)
        for (u32 x = 0; x < 0x80; ++x) {
            const u32 w = (x + (y << 7) + 1) & 0xffff;
            first(map + w * 0x44 - 0x44);
            second(map + w * 0x28 + 0x10ffd8);
        }
}

}  // namespace

void MapSweep_SetEnv(const MapSweepEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall MapSweep_Reset(u8 *map) { sweep(map, g_env->reset, g_env->reset); }
void __fastcall MapSweep_Place(u8 *map) { sweep(map, g_env->place, g_env->placeRide); }
}
