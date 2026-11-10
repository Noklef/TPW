#include "dialog7ca168.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../gfx/barmesh.h"
#include "../math/x87.h"
#include "../world/advisor.h"
#include "../world/ridesweep.h"
#include "../world/thingscan.h"
#include "../world/world.h"
#include "buttontext.h"
#include "dialogs.h"
#include "focus.h"
#include "listrows.h"
#include "panelparts.h"
#include "radiogroup.h"
#include "window.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "windowscript.h"
#include "windowtimers.h"
#include "dialog7ca168handler.h"
#include "panel7b9940.h"
#include "dialog7cc188.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 __cdecl forEachSelected(u32 mask, u32 callback) { return Things_ForEachSelected(mask, reinterpret_cast<ObjectCallback>((uintptr_t)callback)); }

const Dialog7ca168OpenEnv kGameEnv = {&forEachSelected, &Things_AverageStaffStat1f8, &Things_AverageStat1f8OfKind, &Window7059f0_FindItemData, &Window7059f0_ScrollTo};
const Dialog7ca168OpenEnv *g_env = &kGameEnv;

u8 *dialog() { return gDialog7ca168_7ca168; }
}  // namespace

void Dialog7ca168_SetOpenEnv(const Dialog7ca168OpenEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog7ca168_Open(u32 tab, u32 selected) {
    Window7c24c8_Close();
    Global7cb28c_Select(2, 4);
    u8 *const built = WindowScript_Build(gPanel7b9940_7cb2ac, nullptr, gDialog7ca168_Open_750e10, 0x00495da0);
    if (!built) return;
    gDialog7ca168_7ca168 = built;
    Window7c24c8_Set(built);
    if (u8 *const list = Window_FindChild(dialog(), nullptr, 0x321)) {
        Window7059f0_BuildRows(list, nullptr, &List7ca170_NewRow, &List7ca170_InitRow);
        Window7059f0_SetColumnBottom(list, nullptr, 0, 0);
        Window7059f0_SetColumnBottom(list, nullptr, 1, 0);
        Window7059f0_SetColumnBottom(list, nullptr, 2, 1);
        Window7059f0_SetColumnBottom(list, nullptr, 3, 1);
        Window7059f0_SetColumnBottom(list, nullptr, 4, 1);
        Window7059f0_SetHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>((u32)(uintptr_t)&gText_Dialog7cc188_74fa84))));
        Window_SetHandler(list, nullptr, 0x004960a0);
        Window7059f0_SortBy(list, nullptr, gDialog7ca168_75118c, 0);
        for (u32 i = 0; i < 5; ++i) {
            u8 *const button = Window_FindChild(list, nullptr, 0x10 + i);
            ButtonText_Init(button, (u32)(uintptr_t)Language_WideText(0x65 + i), gDialog7ca168_Open_751188, 0xff, 0xef, 0, 0x77 + i);
        }
        const u32 kind = tab == 0xffffffff ? gDialog7ca168_7ca16c : tab;
        gDialog7ca168_7ca16c = kind;
        u8 *const again = Window_FindChild(dialog(), nullptr, 0x321);
        gDialog7ca168_7ca164 = again;
        static const u32 kMasks[5] = {2, 4, 0x10, 8, 0x20};
        if (kind <= 4) g_env->forEach(kMasks[kind], 0x00496320);
        Window7059f0_RefreshRows(again);
        if (selected) {
            const s32 position = g_env->findItem(list, nullptr, selected);
            if (position >= 0) g_env->scrollTo(list, nullptr, position);
        }
        if (u8 *const group = Window_FindChild(list, nullptr, 0x322)) {
            static const u32 kButtons[5] = {0x323, 0x326, 0x325, 0x327, 0x324};
            const u32 shown = gDialog7ca168_7ca16c;
            if (shown <= 4) RadioGroup_Select(group, nullptr, kButtons[shown]);
        }
    }
    u8 *const panel = Window_FindChild(dialog(), nullptr, 0x329);
    panelparts::textNone(panel, 0x32a);
    panelparts::barOf(Window_FindChild(panel, nullptr, 0x32d), panelparts::kHappyGrad, 0, 8);
    panelparts::text(panel, 0x32b, 0x6a);
    u8 *const bar = Window_FindChild(panel, nullptr, 0x32c);
    Window_SaveInsetRect(bar, 0, 8);
    u8 *const last = static_cast<u8 *>(GameHeap_Alloc(0x124));   // (the argument's slot holds it: the kind past the tabs below)
    Window_SetImpl(bar, nullptr, last ? MeshOverlay_Construct(last, nullptr, panelparts::kHappyGrad) : nullptr);
    void *const stats = reinterpret_cast<void *>((uintptr_t)GameTable7cfb90_Get1da720_519510(gWorldPtr));
    const double value = X87_MulDouble(g_env->average(stats, nullptr, 0) * gDialog7ca168_Open_6ff6f4, gDialog7ca168_Open_6ff6f8);
    vcall<void>(bar, 0x1c, (u32)(long long)value);   // (__ftol's low half)
    const u32 shown = gDialog7ca168_7ca16c;
    Dialog7ca168_ShowText(shown);
    u8 *const average = Window_FindChild(Window_FindChild(dialog(), nullptr, 0x329), nullptr, 0x32d);
    static const u32 kKinds[5] = {5, 4, 6, 7, 8};
    const u32 kind = shown <= 4 ? kKinds[shown] : (u32)(uintptr_t)last;
    GameTable7cfb90_Get1da720_519510(gWorldPtr);   // (its answer only the ECX of the stdcall)
    vcall<void>(average, 0x1c, (s32)((u32)g_env->averageOfKind(kind) << 10) / 100);
    if ((u16)Window_SetTimer(dialog(), nullptr, 0x80083, 2000)) Window_StartTimer(dialog(), nullptr, 0x80083);
    Advisor_PostId(0x11d);
}
}
