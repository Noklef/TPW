// How much of a class of the map's cells a kind of staff patrols: see patrolcoverage.h.
#include "patrolcoverage.h"

#include <cstdint>

#include "../core/trivial.h"
#include "mapcell.h"
#include "staff.h"

namespace {

const PatrolCoverageEnv kGameEnv = {&Staff_MarkPatrolCells};
const PatrolCoverageEnv *g_env = &kGameEnv;

bool inGroup(const u8 *cell, u32 group) {
    switch (group) {
    case 0:
        return MapCell_IsType2(cell) || MapCell_IsType7Or1e(cell) || Zero_5d60c0();
    case 1:
        return MapCell_IsType1(cell) || MapCell_IsType3Or9(cell) || MapCell_IsType9(cell) || MapCell_IsTypeA(cell) || Zero_5d60c0() || MapCell_IsType4(cell) ||
               MapHandle_IsTypeBDOr10Or12Or19(reinterpret_cast<const u16 *>(cell)) || MapCell_IsType15(cell);
    case 2:
        return MapCell_IsType9(cell) != 0;
    default:
        return MapCell_IsType1(cell) || MapCell_IsType3Or9(cell);
    }
}

}  // namespace

void PatrolCoverage_SetEnv(const PatrolCoverageEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __stdcall PatrolCoverage_Percent(u32 kind, u32 group) {
    g_env->mark(kind);
    if (group > 3) return 0;
    const u8 *const map = *reinterpret_cast<const u8 *const *>((uintptr_t)0x0080239c) + 0x2d8;
    s32 all = 0, patrolled = 0;
    for (s32 y = 0x7f; y >= 0; --y) {
        for (s32 x = 0x7f; x >= 0; --x) {
            const u8 *const cell = map + 0x44 * ((((x << 7) + y + 1) & 0xffff) - 1);
            if (!inGroup(cell, group)) continue;
            ++all;
            if (*reinterpret_cast<const s32 *>(cell + 0x3c) > 0) ++patrolled;
        }
    }
    return all ? patrolled * 100 / all : 0;
}
