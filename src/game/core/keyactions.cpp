#include "keyactions.h"

#include <cstdint>

#include "../gfx/screengrab.h"
#include "../math/x87.h"
#include "../ui/dialogs.h"
#include "../ui/focus.h"
#include "../ui/guimanager.h"
#include "../world/gamemode.h"
#include "../world/mapcell.h"
#include "../world/mapclick.h"
#include "../world/money.h"
#include "../world/mousemode.h"
#include "../world/research.h"
#include "../world/world.h"
#include "actionrec.h"
#include "gameclock.h"
#include "gameheap.h"
#include "soundplay.h"
#include "globals.h"
#include "globalsave.h"
#include "thunks.h"
#include "vcall.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *gameClock() { return reinterpret_cast<u8 *>((uintptr_t)0x00785970); }

u32 turn(u32 by) {
    Global81d7a4_SetRecorded(static_cast<u32>(static_cast<s32>(Global81d7a4_Get_52f1f0() + by) % 360), 1);
    return 1;
}

const KeyCancelEnv kCancelPorts = {&Obj7b51f0_ResetDialogChild21, &GameMode_Set, &Sound_PlayAt, &GameMode_MapClick, &GameMode_StepBack};
const KeyCancelEnv *g_cancelEnv = &kCancelPorts;

u32 currentModeKind() {
    u8 *const current = global<u8 *>(0x007b05d8);
    return current ? vcall<u32>(current, 0x24) : 0;
}

}  // namespace

void KeyCancel_SetEnv(const KeyCancelEnv *env) { g_cancelEnv = env ? env : &kCancelPorts; }

extern "C" {

u32 __cdecl KeyAction_ClickCell() {
    MouseModes_EnsureHolder();
    const u32 kind = currentModeKind();
    if (Global81ae2c_Equals(0) != 0 && (kind == 0 || kind == 1)) {
        const u32 id = global<u16>(0x007b05cc);
        const s32 column = static_cast<s32>((id - 1) & 0x7f), row = static_cast<s32>((id - 1) >> 7);
        // (The original reads the game mode once more here, 0x52f840, the answer unused.)
        if (MapCell_IsType1(global<u8 *>(0x007cf83c) + id * 0x44 + 0x294) == 0) return 1;
        const s32 z = (s32)(long long)(X87Reg((double)row) * 10.0f).v;   // (0x6fd764; fild, fmul, __ftol: the row's first)
        const s32 x = (s32)(long long)(X87Reg((double)column) * 10.0f).v;
        g_cancelEnv->playAt(0, global<u32>(0x00803a2c), 0x5f, x, 0, z);
        g_cancelEnv->setMode(0x32, 1);
        g_cancelEnv->mapClick(column, row, 0, 0);
        g_cancelEnv->setMode(0, 1);
        return 1;
    }
    if (kind == 3) g_cancelEnv->stepBack(nullptr, 1);
    return 1;
}

u32 __cdecl KeyAction_CancelMouseMode() {
    if (g_cancelEnv->reset(reinterpret_cast<void *>((uintptr_t)0x007b51f0))) return 1;
    MouseModes_EnsureHolder();
    if (currentModeKind() == 0) return 0;
    MouseModes_EnsureHolder();
    if (currentModeKind() == 1) return 0;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
    u8 *const mode = block ? MouseModeOnlineBase_Ctor(block) : nullptr;
    MouseModes_EnsureHolder();
    MouseMode_Set(mode);
    g_cancelEnv->setMode(0, 1);
    return 1;
}

u32 __cdecl KeyAction_Turn90() { return turn(0x5a); }
u32 __cdecl KeyAction_Turn270() { return turn(0x10e); }

u32 __cdecl KeyAction_ClockSlower() {
    GameClock_Slower(gameClock());
    return 1;
}

u32 __cdecl KeyAction_ClockFaster() {
    GameClock_Faster(gameClock());
    return 1;
}

u32 __cdecl KeyAction_ClockNormal() {
    GameClock_SetRate(gameClock(), nullptr, 0, 0x3ff00000);
    return 1;
}

u32 __cdecl KeyAction_Credit10000() {
    Money_Credit(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(global<u8 *>(0x007cf83c))), nullptr, 10000);
    return 1;
}

u32 __cdecl KeyAction_Toggle787d1c() {
    const u32 v = 1 - global<u32>(0x00787d1c);
    global<u32>(0x00787d1c) = v;
    Global878128_Set_550510(v);
    return 1;
}

u32 __cdecl KeyAction_ViewFillStep() {
    u32 &flags = global<u32>(0x008bcbc8);
    switch (flags & 0xc) {
    case 4: flags &= ~0xcu; break;
    case 0xc: flags = (flags & ~8u) | 4; break;
    default: flags |= 0xc; break;
    }
    return 1;
}

u32 __cdecl KeyAction_ShowVersion() {
    Wrap_MsgTags_AddVersion_481b70();
    return 1;
}

u32 __cdecl KeyAction_ScreenGrab() {
    ScreenGrab_NextName();
    return 1;
}

u32 __cdecl KeyAction_MarkTheme0() { GlobalSave_MarkTheme(GlobalSave_Instance(), nullptr, 0); return 1; }
u32 __cdecl KeyAction_MarkTheme1() { GlobalSave_MarkTheme(GlobalSave_Instance(), nullptr, 1); return 1; }
u32 __cdecl KeyAction_MarkTheme2() { GlobalSave_MarkTheme(GlobalSave_Instance(), nullptr, 2); return 1; }
u32 __cdecl KeyAction_MarkTheme3() { GlobalSave_MarkTheme(GlobalSave_Instance(), nullptr, 3); return 1; }
u32 __cdecl KeyAction_MarkTheme4() { GlobalSave_MarkTheme(GlobalSave_Instance(), nullptr, 4); return 1; }
u32 __cdecl KeyAction_MarkTheme5() { GlobalSave_MarkTheme(GlobalSave_Instance(), nullptr, 5); return 1; }

u32 __cdecl KeyAction_MarkScore0() { GlobalSave_MarkScore(GlobalSave_Instance(), nullptr, 0, 100); return 1; }
u32 __cdecl KeyAction_MarkScore1() { GlobalSave_MarkScore(GlobalSave_Instance(), nullptr, 1, 100); return 1; }
u32 __cdecl KeyAction_MarkScore2() { GlobalSave_MarkScore(GlobalSave_Instance(), nullptr, 2, 100); return 1; }
u32 __cdecl KeyAction_MarkScore3() { GlobalSave_MarkScore(GlobalSave_Instance(), nullptr, 3, 100); return 1; }

u32 __cdecl KeyAction_FlagResearchControls() {
    ResearchLab_FlagProjectControls(reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(global<u8 *>(0x007cf83c))));
    return 1;
}

u32 __cdecl KeyAction_ToggleOverlay() {
    Obj7b51f0_ToggleOverlay(reinterpret_cast<void *>((uintptr_t)0x007b51f0));
    return 1;
}

u32 __cdecl KeyAction_GuiManagerSlotAndFlag() {
    Obj7b51f0_GuiManagerSlotAndFlag(reinterpret_cast<void *>((uintptr_t)0x007b51f0));
    return 1;
}
}
