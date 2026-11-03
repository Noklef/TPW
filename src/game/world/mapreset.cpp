// The map's cells reset at a new park: see mapreset.h.
#include "mapreset.h"

#include <cstdint>

#include "cellkind.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const MapResetEnv kGameEnv = {MapCell_Reset, TrackCell_Reset};
const MapResetEnv *g_env = &kGameEnv;

}  // namespace

void MapReset_SetEnv(const MapResetEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Map_ResetCells(s32 columns, s32 rows) {
    for (s32 row = 0; row < rows; row++) {
        for (s32 column = 0; column < columns; column++) {
            const u32 id = (u16)((row << 7) + column + 1);
            g_env->mapReset(global<u8 *>(0x007cf83c) + id * 0x44 + 0x294, nullptr, 1);
            g_env->trackReset(global<u8 *>(0x007cf83c) + (id + 0x6cde) * 0x28);
        }
    }
}
