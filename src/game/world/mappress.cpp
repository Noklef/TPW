#include "mappress.h"

#include <cstdint>

#include "../coaster/edittools.h"
#include "../core/actionrec.h"
#include "../core/globals.h"
#include "cellmessage.h"
#include "cellrun.h"
#include "focuscell.h"
#include "mappreview.h"
#include "mapclick.h"
#include "focuscell.h"
#include "cellbuild.h"
#include "pathdrag.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const MapPressEnv kGame = {GameMode_MapPreview, ActionRec_WriteBB, ActionRec_WriteBBW, FocusCell_Update,
                           PairTable_Add,       Map_SendLine,      CoasterEdit_PlaceSelected, CoasterEdit_Click};
const MapPressEnv *g_env = &kGame;

}  // namespace

void MapPress_SetEnv(const MapPressEnv *env) { g_env = env ? env : &kGame; }

extern "C" void __cdecl GameMode_MapPress(s32 x, s32 y, u32 flags, u32 useStored) {
    gGameMode_8186d8 = 1;
    if (global<u32>(0x00785a2c) != 0) {
        gCellbuild_816d50 = 0;
        if (gMapclick_8187f8 == 0) g_env->preview(x, y, flags);
    }
    if (useStored != 0) {
        x = gFocuscell_8186b8;
        y = gFocuscell_8186b4;
    }
    ActionRec *const record = reinterpret_cast<ActionRec *>((uintptr_t)0x00785a08);
    if (gFocuscell_816d48 == 0) {
        if ((u16)flags == 0) g_env->writeBB(record, nullptr, 0xf, (u32)x, (u32)y);
        else g_env->writeBBW(record, nullptr, 0x28, (u32)x, (u32)y, flags);
    }
    if (Global81ae2c_Equals(0x38) == 0 && gFocuscell_816d48 != 0) return;
    if (Global81ae2c_Equals(0) != 0) return;
    if (Global81ae2c_Equals(0x16) == 0) return;
    g_env->focusUpdate(0);
    if (gPathdrag_818c54 == 0) {
        g_env->click(1);
        return;
    }
    g_env->pairAdd((u32)x, (u32)y);
    g_env->sendLine(0x87, x, y, &x, &y);
    g_env->sendLine(0x15, x, y, &x, &y);
    g_env->sendLine(0x83, x, y, &x, &y);
    g_env->sendLine(0x81, x, y, &x, &y);
    const u32 cell = (((u32)y << 7) + (u32)x + 1) & 0xffff;
    const u32 placed = g_env->placeSelected(x, y);
    *reinterpret_cast<u32 *>((uintptr_t)global<u32>(0x007cf83c) + (cell + 0x6cde) * 0x28 + 4) = placed;
    global<u32>(0x007639a0) = 0xffffffff;
    gPathdrag_818c54 = 0;
}
