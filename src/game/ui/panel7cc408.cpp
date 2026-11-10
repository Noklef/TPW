#include "panel7cc408.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/soundplay.h"
#include "../render/textparts.h"
#include "../world/advisor.h"
#include "../world/ridesweep.h"
#include "dialogs.h"
#include "guirect.h"
#include "panelparts.h"
#include "thingpreview.h"
#include "window.h"
#include "panel7cc218.h"
#include "../core/soundset.h"

using namespace panelparts;

namespace {

const Panel7cc408Env kGameEnv = {&Things_CountGroup, &TextPart_Ctor, &Window_SetImpl, &Window_NotifyC00, &Window_Notify3000, &Window_Set4c, &Window_Notify40000, &Window_NotifyImpl100};
const Panel7cc408Env *g_env = &kGameEnv;
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
}  // namespace

void Panel7cc408_SetEnv(const Panel7cc408Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall Panel7cc408_Refresh(u8 *self) {
    g_env->countGroup(self, nullptr, *reinterpret_cast<const u32 *>((uintptr_t)0x007cc430));
    u8 *const child = Window_FindChild(*reinterpret_cast<u8 **>(self + 0x10), nullptr, 0x3734);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    g_env->setImpl(child, nullptr, memory ? g_env->partText(memory, nullptr, 0) : nullptr);
    g_env->notifyC00(child, nullptr, 1, 1);
    g_env->notify3000(child, nullptr, 1, 1);
    g_env->set4c(child, nullptr, Language_Font(6));
    g_env->notify40000(child, nullptr, 0, 0, 0, 0xff);
    g_env->notifyImpl100(child, nullptr, 0);
}
}

extern "C" {

void __fastcall Panel7cc408_ShowPreview(u8 *self) {
    if (!*reinterpret_cast<u8 *const *>(&gPanel7cc218_7c2658)) return;
    u8 rect[0xc];
    GuiRect_Ctor(rect);
    u8 *const shown = *reinterpret_cast<u8 **>(self + 0x18);
    Window_GetSavedRect(shown, nullptr, rect);
    ThingPreview_Show(rect, *reinterpret_cast<u8 *const *>(&gPanel7cc218_7c2658), *reinterpret_cast<const s16 *>(shown + 0xe0) + 2, 0x41400000);
    GuiRect_Dtor(rect);
}

void __fastcall Panel7cc408_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007cc430) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x007575e0), 0x004b7720, 0x3722, 0x3723, 0);
    Sound_PlayAt(0, gSoundCategories.kids, 0x81, 0, 0, 0);
    Advisor_PostId(0x12e);
}

void __fastcall Panel7cc408_Dress(u8 *self) {
    if (!window(self)) return;
    text(window(self), 0x372c, 0x46);
    text(window(self), 0x3725, 0x47);
    text(window(self), 0x3726, 0x48);
    text(window(self), 0x3727, 0x4b);
    text(window(self), 0x3728, 0x49);
    text(window(self), 0x3729, 0x4a);
    text(window(self), 0x372a, 0x4c);
    text(window(self), 0x372b, 0x4d);
    part(window(self), TextPart6ff3b0_Ctor, 0x3724, 0x29a);
    part(window(self), TextPart6ff4a0_Ctor, 0x372d);
    part(window(self), TextPart6ff350_Ctor, 0x372e, 0);
    part(window(self), TextPart6ff350_Ctor, 0x372f, 0);
    part(window(self), TextPart6ff350_Ctor, 0x3730, 0);
    part(window(self), TextPart6ff350_Ctor, 0x3731, 0);
    barIfAny(window(self), kHappyGrad, 0x3732);
    textNone(window(self), 0x3733);
}
}
