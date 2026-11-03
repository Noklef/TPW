#include "dialog7ca210.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/language.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "../world/advisor.h"
#include "../world/world.h"
#include "focus.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "window.h"
#include "window705a50.h"
#include "windowscript.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *dialog() { return global<u8 *>(0x007ca210); }

const Dialog7ca210Env kGameEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), &HoverLabel_Attach,
                                  reinterpret_cast<void(__fastcall *)(u8 *, void *, u8 *, u32)>((uintptr_t)0x004e2780)};
const Dialog7ca210Env *g_env = &kGameEnv;

}  // namespace

void Dialog7ca210_SetEnv(const Dialog7ca210Env *env) { g_env = env ? env : &kGameEnv; }

u8 *__cdecl Thing_NameBuffer7ba058(u8 *thing) {
    u8 *const buffer = reinterpret_cast<u8 *>((uintptr_t)0x007ba058);
    g_env->copyName(thing, nullptr, buffer, 0x3fff);
    return buffer;
}

void __cdecl Dialog7ca210_Open() {
    if (dialog()) return;
    Window7c24c8_Close();
    Global7cb28c_Select(3, 10);
    global<u8 *>(0x007ca210) = WindowScript_Build(global<u8 *>(0x007cb2ac), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00751798), 0x00498c60);
    Window7c24c8_Set(dialog());
    Window_SetHandler(Window_FindChild(dialog(), nullptr, 0x4f3af), nullptr, 0x00499060);
    const u16 word = *reinterpret_cast<const u16 *>(global<u8 *>(0x007cf83c) + 0x1da732);
    u8 *const name = Thing_NameBuffer7ba058(global<u8 *>(0x007cfb90 + 20 * word));
    g_env->attach(dialog(), 0x4f3af, (u32)(uintptr_t)name);
    u8 *child = panelparts::withPart(dialog(), TextPart_Ctor, 0x4f3b0);
    Window_NotifyC00(child, nullptr, 1, 1);
    panelparts::font6(child);
    Window_Notify40000(child, nullptr, 0, 0, 0, 0xff);
    Window_NotifyImpl100(child, nullptr, (u32)(uintptr_t)Language_WideText(0xa0));
    child = Window_FindChild(dialog(), nullptr, 0x4f3ae);
    panelparts::rangedNumber(child, 1);
    const u8 *const money = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(global<u8 *>(0x007cf83c)));
    const u32 value = *reinterpret_cast<const u32 *>(money + 0x118);
    global<u32>(0x007ca1e8) = value;
    vcall<void>(child, 0x1c, value);
    child = Window_FindChild(dialog(), nullptr, 0x4f3b1);
    const u32 flag = Get1da710_51a280(global<u8 *>(0x007cf83c));
    global<u32>(0x007ca208) = flag;
    Window705a50_SetState(child, nullptr, flag ? 1 : 0, 1);
    u8 &guard = global<u8>(0x00fb34c5);
    if (!(guard & 1)) {
        guard |= 1;
        Setting_ReadMode(reinterpret_cast<u8 *>((uintptr_t)0x00fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (global<u32>(0x00fb3b7c) == 2) vcall<void>(Window_FindChild(dialog(), nullptr, 0x4f3ac), 0x14, 0u);
    Advisor_PostId(0x12f);
}
