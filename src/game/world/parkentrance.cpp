#include "parkentrance.h"

#include <cstdint>

#include "../terrain/normals.h"
#include "cellmodel.h"
#include "cellrun.h"
#include "focuscell.h"
#include "gamemode.h"
#include "mapcell.h"
#include "mapclick.h"
#include "trackline.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const ParkEntranceEnv kPortEnv = {&GameMode_Set, &FocusCell_Update, &GameMode_MapClick, &CellRun_Mark, &TrackLine_Prune, &CellRun_Lay, &MapCell_ApplyPathModel, &Terrain_ShadeNormals};
const ParkEntranceEnv *g_env = &kPortEnv;

s32 &EndColumn() { return global<s32>(0x007639f0); }
s32 &EndRow() { return global<s32>(0x007639f4); }
s32 &StartColumn() { return global<s32>(0x007639f8); }
s32 &StartRow() { return global<s32>(0x007639fc); }

void Click(s32 x, s32 y) { g_env->mapClick(x, y, 0, 0); }

void ClearCell(u32 id) {
    const u32 index = id & 0xffff;
    u8 *const cell = global<u8 *>(0x007cf83c) + index * 0x44 + 0x294;
    MapCell_ClearFlags(cell, nullptr, 0x40);
    g_env->pathModel(cell);
}

}  // namespace

void ParkEntrance_SetEnv(const ParkEntranceEnv *env) { g_env = env ? env : &kPortEnv; }

void __cdecl ParkEntrance_Lay() {
    u16 first, second;
    global<u8>(0x0081b4cc) = 1;
    Map_EntrancePos(global<u8 *>(0x007cf83c) + 0x2d8, nullptr, &first, 0);
    Map_EntrancePos(global<u8 *>(0x007cf83c) + 0x2d8, nullptr, &second, 1);

    global<u8>(0x00818c50) = 4;
    const u32 start = (u32)first - 1, end = (u32)second - 1;
    StartRow() = (s32)(start >> 7);
    StartColumn() = (s32)(start & 0x7f) - 4;
    EndColumn() = (s32)(end & 0x7f) + 3;
    EndRow() = (s32)(end >> 7);
    g_env->setMode(0x19, 1);
    g_env->focusUpdate(1);

    if (global<u32>(0x0079d86c) & 0x400) {
        s32 x = EndColumn(), y = EndRow();
        Click(x, y);
        Click(x += 2, y);
        Click(x, y += 4);
        Click(x -= 0xc, y);
        Click(x, y -= 4);
        Click(x + 2, y);
    } else if (global<s32>(0x00784e3c) != 0x200) {
        for (const s32 *pair = &global<s32>(0x00784e3c); pair < &global<s32>(0x0078503c); pair += 2) {
            Click(pair[0] + EndColumn(), pair[1] + EndRow());
            if (pair[2] == 0x200)
                break;
        }
    }

    g_env->mark(1);
    g_env->prune();
    g_env->mark(0);
    g_env->lay(EndColumn(), EndRow(), StartColumn(), StartRow(), 4, 1);
    g_env->setMode(0, 1);

    for (s32 column = StartColumn() + 2; column < EndColumn(); ++column) {
        ClearCell(((u32)EndRow() << 7) + column + 1);
        ClearCell(((u32)EndRow() << 7) + column + 0x81);
    }
    global<u8>(0x0081b4cc) = 0;
    g_env->shade();
}
