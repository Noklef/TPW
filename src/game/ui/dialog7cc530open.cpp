// The online dialog's dial-up page: see dialog7cc530open.h.
#include "dialog7cc530open.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../render/textparts.h"
#include "buttontext.h"
#include "dialogbase.h"
#include "dialuplist.h"
#include "hoverlabel.h"
#include "radiogroup.h"
#include "window.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "dialog7cc530handler.h"
#include "dialog7cc188.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7cc530OpenEnv kGameEnv = {&Dialog_Create, &HoverLabel_SetYellow, &RadioGroup_Select, &Window7059f0_BuildRows, &Window7059f0_SetColumnBottom, &GuiWindows_FindByName, &Window7059f0_SetHighlight, &ButtonText_Init, &DialupList_Fill, &Language_WideText};
const Dialog7cc530OpenEnv *g_env = &kGameEnv;

void dressButton(u8 *group, u32 id, u32 text) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    u8 *const part = memory ? TextPart_Ctor(memory, nullptr, 0) : nullptr;
    Window_SetImpl(Window_FindChild(group, nullptr, id), nullptr, part);
    Window_NotifyC00(Window_FindChild(group, nullptr, id), nullptr, 1, 1);
    Window_Notify3000(Window_FindChild(group, nullptr, id), nullptr, 1, 1);
    u8 *const font = Language_Font(6);
    Window_Set4c(Window_FindChild(group, nullptr, id), nullptr, font);
    Window_Notify40000(Window_FindChild(group, nullptr, id), nullptr, 0xff, 0xff, 0xff, 0xff);
    u16 *const wide = g_env->wideText(text);
    Window_NotifyImpl100(Window_FindChild(group, nullptr, id), nullptr, (u32)(uintptr_t)wide);
}

void header(u8 *list, u32 id, u32 text) {
    u8 *const child = Window_FindChild(list, nullptr, id);
    g_env->buttonText(child, (u32)(uintptr_t)g_env->wideText(text), 8, 0xff, 0xef, 0, 0xffffffffu);
}

}  // namespace

void Dialog7cc530_SetOpenEnv(const Dialog7cc530OpenEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Dialog7cc530_Open(u32 value) {
    gDialog7cc530open_7cc52c = value;
    u8 *const built = g_env->create(reinterpret_cast<u8 *>((uintptr_t)&gDialog7cc530_Open_758218), 0x004ba530, 0);
    gDialog7cc530open_7cc530 = built;
    gDialog7cc530handler_7cc53c = 0;
    gDialog7cc530open_7cc540 = 0;
    u8 *const title = Window_FindChild(built, nullptr, 0xb);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(title, nullptr, memory ? TextPart6ff530_Ctor(memory, nullptr, 0) : nullptr);
    Window_Set4c(title, nullptr, Language_Font(5));
    Window_NotifyC00(title, nullptr, 1, 1);
    vcall<void>(title, 0x24, (u32)(uintptr_t)g_env->wideText(0xe1));
    g_env->yellow(title);
    u8 *const group = Window_FindChild(gDialog7cc530open_7cc530, nullptr, 0xd);
    dressButton(group, 0xe, 0xe5);
    dressButton(group, 0xf, 0xe4);
    g_env->select(group, nullptr, 0xe);
    u8 *const list = Window_FindChild(gDialog7cc530open_7cc530, nullptr, 0xc);
    g_env->buildRows(list, nullptr, GameAddress<u8 *(__cdecl *)()>(0x004bacc0), reinterpret_cast<void(__cdecl *)(u8 *)>((uintptr_t)0x004bad70));
    g_env->setColumn(list, nullptr, 0, 0);
    g_env->setColumn(list, nullptr, 1, 1);
    g_env->setHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_env->findByName(reinterpret_cast<const char *>((uintptr_t)&gText_Dialog7cc188_74fa84))));
    header(list, 0x10, 0xe2);
    header(list, 0x11, 0xe3);
    g_env->fill(0);
}
