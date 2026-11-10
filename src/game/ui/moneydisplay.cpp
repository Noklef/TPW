#include "moneydisplay.h"

#include <cstdint>

#include "../coaster/registry.h"
#include "../core/gameheap.h"
#include "../core/globals.h"
#include "../core/vcall.h"
#include "../files/wrapsources.h"
#include "../world/mousemode.h"
#include "../world/world.h"
#include "guirect.h"
#include "window.h"
#include "windowarea.h"
#include "panel7b9940.h"
#include "msgtags.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const s32 kNoCost = -999;

u32 __fastcall AmountGame(u8 *money) { return Source_Field0c(reinterpret_cast<const TrackedSource *>(money)); }
void __fastcall SetModeGame(u8 *, void *, u8 *mode) { MouseMode_Set(mode); }

const MoneyDisplayEnv kGameEnv = {
    &GameTable7cfb90_Get1da726_5195d0,
    &AmountGame,
    &Window_Notify40000,
    &Window_GetAreaRect,
    &Window_MoveTo,
    &Window_GetRect,
    &Window_Notify1,
    &MouseModeOnlineBase_Ctor,
    &SetModeGame,
    &CoasterEdit_Flag1,
    &Global81af4c_Get_52fbc0,
    &Window_SetShown,
};
const MoneyDisplayEnv *g_env = &kGameEnv;

void ShowMoney(u8 *window, s32 money) {
    const MoneyDisplayEnv *const E = g_env;
    s32 index = (money + 999) / 1000;
    if (index < 0) index = 0;
    if (index > 5) index = 5;
    gMoneydisplay_7cb27c = index;
    if (index != gMoneydisplay_752e2c) {
        const u32 entry = (u32)(uintptr_t)&gMsgtags_752e30 + 4 * static_cast<u32>(index);
        E->notify40000(window, nullptr, global<u8>(entry), global<u8>(entry + 1), global<u8>(entry + 2), 0xff);
        gMoneydisplay_752e2c = gMoneydisplay_7cb27c;
    }
    vcall<void>(window, 0x1c, static_cast<u32>(money < 0 ? -money : money));
    u8 area[0x18], own[0x18], cost[0x18];
    GuiRect_Ctor(area);
    GuiRect_Ctor(own);
    GuiRect_Ctor(cost);
    E->areaRect(window, nullptr, area);
    E->moveTo(Window_FindChild(window, nullptr, 0x31), nullptr, reinterpret_cast<const s16 *>(area + 8), nullptr);
    u8 *const child = Window_FindChild(window, nullptr, 0x30);
    E->getRect(window, nullptr, own, 1);
    E->getRect(child, nullptr, cost, 1);
    const u32 x = at<u32>(cost, 4) - at<u32>(cost, 8) + at<u32>(area, 8);
    E->moveTo(child, nullptr, reinterpret_cast<const s16 *>(&x), nullptr);
    E->notify1(Window_FindChild(window, nullptr, 0x32), nullptr, money < 0 ? 1 : 0);
    gMoneydisplay_7cb2dc = money;
    GuiRect_Dtor(cost);
    GuiRect_Dtor(own);
    GuiRect_Dtor(area);
}

void EnsureHolder() {
    const MoneyDisplayEnv *const E = g_env;
    if (gPanel7b9940_7b05e8) return;
    u8 *const holder = static_cast<u8 *>(GameHeap_Alloc(1));
    if (holder) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
        E->setMode(holder, nullptr, block ? E->modeBase(block) : nullptr);
        global<s32>(0x007b05e0) = -1;
        global<s32>(0x007b05d4) = -1;
    }
    gPanel7b9940_7b05e8 = holder;
}

}  // namespace

extern "C" {

void MoneyDisplay_SetEnv(const MoneyDisplayEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl MoneyDisplay_Update(u8 *window) {
    const MoneyDisplayEnv *const E = g_env;
    const s32 money = static_cast<s32>(E->amount(reinterpret_cast<u8 *>((uintptr_t)E->money(*global<u8 **>(0x007cf6ec)))));
    if (money != gMoneydisplay_7cb2dc) ShowMoney(window, money);
    EnsureHolder();

    u8 *const mode = global<u8 *>(0x007b05d8);
    const u32 kind = mode ? vcall<u32>(mode, 0x24) : 0;
    if (kind == 3 || kind == 4 || (kind == 8 && E->editFlag())) {
        const s32 cost = static_cast<s32>(E->cost());
        if (cost == gMoneyDisplay_Update_752ec0) return;
        u8 *const child = Window_FindChild(window, nullptr, 0x30);
        if (!child) return;
        if (cost != kNoCost) E->setShown(child, nullptr, 1);
        if (cost == -9999) {
            E->setShown(child, nullptr, 0);
            return;
        }
        vcall<void>(child, 0x1c, static_cast<u32>(cost));
        gMoneyDisplay_Update_752ec0 = cost;
        return;
    }
    if (gMoneyDisplay_Update_752ec0 == kNoCost) return;
    if (u8 *const child = Window_FindChild(window, nullptr, 0x30)) E->setShown(child, nullptr, 0);
    gMoneyDisplay_Update_752ec0 = kNoCost;
}
}
