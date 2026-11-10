// The dialog [0x7cca70]: see dialog7cca70.h.
#include "dialog7cca70.h"

#include <cstdint>

#include "../core/language.h"
#include "../files/charsets.h"
#include "../frontend/scenequeue.h"
#include "../online/session.h"
#include "buttontext.h"
#include "hoverlabel.h"
#include "window.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "windowscript.h"
#include "../core/textarg.h"
#include "dialog7cc188.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

void __fastcall Request(u8 *object, void *, u32 a, u32 callback, u32 b) { reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32, u32)>((uintptr_t)0x005bcd10)(object, nullptr, a, callback, b); }
s32 __cdecl FormatWide(u8 *charset, u16 *dest, u32 size, u32 entry, const u32 *arg) { return Charset_FormatWide(charset, dest, size, entry, arg); }

const Dialog7cca70Env kGameEnv = {&Request, &Window_SetFocus, &Window7059f0_BuildRows, &Window7059f0_SetColumnBottom, &Window7059f0_SetHighlight, &ButtonText_Init, &FormatWide, &HoverLabel_Attach, &Class702da0_Push};
const Dialog7cca70Env *g_env = &kGameEnv;

}  // namespace

void Dialog7cca70_SetEnv(const Dialog7cca70Env *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Dialog7cca70_Open(u32 value) {
    u8 *const built = WindowScript_Build(at<u8 *>(global<u8 *>(0x00f82884), 4), nullptr, gDialog7cca70_Open_7593e8, 0x004c1a10);
    gDialog7cca70_7cca8c = value;
    global<u8 *>(0x007cca70) = built;
    u8 *const list = Window_FindChild(built, nullptr, 0x14beb);
    g_env->buildRows(list, nullptr, reinterpret_cast<u8 *(__cdecl *)()>((uintptr_t)0x004c1b30), reinterpret_cast<void(__cdecl *)(u8 *)>((uintptr_t)0x004c1c30));
    for (u32 column = 0; column < 5; ++column) g_env->setColumn(list, nullptr, column, column >= 2);
    g_env->setHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>((u32)(uintptr_t)&gText_Dialog7cc188_74fa84))));
    for (u32 i = 0; i < 5; ++i) {
        const u16 *const text = Language_WideText(0x126 + i);
        g_env->buttonText(Window_FindChild(list, nullptr, 0x10 + i), (u32)(uintptr_t)text, 8, 0xff, 0xef, 0, 0x177 + i);
    }
    const u32 a = at<u32>(global<u8 *>(0x00f82888), 8);
    g_env->request(at<u8 *>(Online_Instance(), 0xe8), nullptr, a, 0x004c20c0, 0);
    const u32 arg[4] = {(u32)(uintptr_t)TextArgString_vtable, 0x5641524d, 0xf, at<u32>(global<u8 *>(0x00f82888), 0x14)};
    g_env->format(global<u8 *>(0x0078ba94), reinterpret_cast<u16 *>((uintptr_t)0x007ba058), 0x3fff, 0x1bf, arg);
    g_env->attach(global<u8 *>(0x007cca70), 0x14bec, 0x007ba058);
    g_env->setFocus(global<u8 *>(0x007cca70), nullptr, 0);
    g_env->push(at<u8 *>(global<u8 *>(0x00f85614), 0x14), nullptr, 0x253, 1);
}
