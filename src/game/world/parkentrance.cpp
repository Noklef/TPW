#include "parkentrance.h"

#include <cstdint>

#include "../terrain/normals.h"
#include "../ui/cursor.h"
#include "cellmodel.h"
#include "cellrun.h"
#include "focuscell.h"
#include "gamemode.h"
#include "mapcell.h"
#include "mapclick.h"
#include "trackline.h"
#include "world.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const ParkEntranceEnv kPortEnv = {&GameMode_Set, &FocusCell_Update, &GameMode_MapClick, &CellRun_Mark, &TrackLine_Prune, &CellRun_Lay, &MapCell_ApplyPathModel, &Terrain_ShadeNormals, &GameCursor_Set};
const ParkEntranceEnv *g_env = &kPortEnv;

s32 &EndColumn() { return gFocusCell_PointAX_7639f0; }
s32 &EndRow() { return gFocusCell_PointAY_7639f4; }
s32 &StartColumn() { return gFocusCell_PointBX_7639f8; }
s32 &StartRow() { return gFocusCell_PointBY_7639fc; }

void Click(s32 x, s32 y) { g_env->mapClick(x, y, 0, 0); }

void ClearCell(u32 id) {
    const u32 index = id & 0xffff;
    u8 *const cell = gWorldPtr + index * 0x44 + 0x294;
    MapCell_ClearFlags(cell, nullptr, 0x40);
    g_env->pathModel(cell);
}

}  // namespace

void ParkEntrance_SetEnv(const ParkEntranceEnv *env) { g_env = env ? env : &kPortEnv; }

void __cdecl ParkEntrance_Lay() {
    u16 first, second;
    global<u8>(0x0081b4cc) = 1;
    Map_EntrancePos(gWorldPtr + 0x2d8, nullptr, &first, 0);
    Map_EntrancePos(gWorldPtr + 0x2d8, nullptr, &second, 1);

    gFocuscell_818c50 = 4;
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
    } else if (global<s32>((u32)(uintptr_t)&gBalanceHoardingsA + 0xc) != 0x200) {
        for (const s32 *pair = &global<s32>((u32)(uintptr_t)&gBalanceHoardingsA + 0xc); pair < &global<s32>((u32)(uintptr_t)&gBalanceHoardingsA + 0x20c); pair += 2) {
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

void __cdecl ParkEntrance_LayOutline(s32 x1, s32 y1, s32 x2, s32 y2) {
    const s32 left = x1 < x2 ? x1 : x2, right = x1 > x2 ? x1 : x2;
    const s32 top = y1 < y2 ? y1 : y2, bottom = y1 > y2 ? y1 : y2;
    global<u8>(0x0081b4cc) = 1;
    g_env->setMode(0x19, 0);
    Click(left, top);
    Click(right, top);
    Click(right, bottom);
    Click(left, bottom);
    Click(left, top);
    g_env->setMode(0, 0);
    g_env->cursor(1);
    gMapClick_7639c0 = g_env->lay(EndColumn(), EndRow(), StartColumn(), StartRow(), 4, 1);
    g_env->mark(1);
    g_env->prune();
    g_env->mark(0);
    g_env->shade();
    global<u8>(0x0081b4cc) = 0;
}
