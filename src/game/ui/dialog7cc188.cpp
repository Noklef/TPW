// The list of the dialog [0x7cc188]: see dialog7cc188.h.
#include "dialog7cc188.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/options.h"
#include "../core/stdlist.h"
#include "../core/trivial.h"
#include "../core/vcall.h"
#include "../files/userslots.h"
#include "../online/session.h"
#include "../render/textparts.h"
#include "../world/advisor.h"
#include "hoverlabel.h"
#include "window.h"
#include "window705a20.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "windowscript.h"
#include "dialog7cc188handler.h"
#include "dialog7cc184handler.h"
#include "dialog74fb20.h"
#include "gamepause.h"
#include "window7cc8c0handler.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

s32 __cdecl Swprintf(u16 *to, const u16 *format, const char *text) { return Crt().swprintf(to, format, text); }

const Dialog7cc188ListEnv kGameEnv = {&UserSlots_Instance,&Swprintf, GameAddress<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0), &Window7059f0_Clear, &Window7059f0_AddItem, &Window7059f0_RefreshRows, &Language_WideText};
const Dialog7cc188ListEnv *g_env = &kGameEnv;

const char *textOf(const StdString *string) {
    const char *const text = at<const char *>(string, 4);
    return text ? text : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0);
}

const Dialog7cc188OpenEnv kGameOpenEnv = {WindowScript_Build, GameHeap_Alloc, TextPart6ff530_Ctor, TextPart6ff4d0_Ctor, Window_SetImpl, Language_Font, Window_Set4c, Window_NotifyC00, Language_WideText, HoverLabel_SetYellow,
                                          Window7059f0_BuildRows, Window7059f0_SetColumnBottom, GuiWindows_FindByName, Window7059f0_SetHighlight, Dialog7cc188_FillList, Window_Notify40000, Window_SetHandler,
                                          Window705a20_SetBuffer, Window705a20_SetCaret, Advisor_PostId};
const Dialog7cc188OpenEnv *g_open = &kGameOpenEnv;

}  // namespace

void Dialog7cc188_SetListEnv(const Dialog7cc188ListEnv *env) { g_env = env ? env : &kGameEnv; }
void Dialog7cc188_SetOpenEnv(const Dialog7cc188OpenEnv *env) { g_open = env ? env : &kGameOpenEnv; }

void __cdecl Dialog7cc188_Open() {
    u8 *&window = gDialog7cc188handler_7cc188;
    if (window) return;
    window = g_open->build(gDialog7cc184handler_7cc184, nullptr, gDialog7cc188_Open_7542b8, 0x004a73d0);
    u8 *const label = static_cast<u8 *>(g_open->alloc(0x1a4));
    u8 *const labelPart = label ? g_open->labelPart(label, nullptr, 0) : nullptr;
    g_open->setImpl(Window_FindChild(window, nullptr, 0x27), nullptr, labelPart);
    u8 *const labelFont = g_open->font(5);
    g_open->set4c(Window_FindChild(window, nullptr, 0x27), nullptr, labelFont);
    g_open->notifyC00(Window_FindChild(window, nullptr, 0x27), nullptr, 1, 1);
    u8 *const child = Window_FindChild(window, nullptr, 0x27);
    vcall<void>(child, 0x24, g_open->text(0xd4));
    g_open->hoverYellow(Window_FindChild(window, nullptr, 0x27));
    Window_SetDepth(window, nullptr, (u16)(at<u16>(gDialog7cc184handler_7cc184, 0xe0) + 0xa));
    u8 *const list = Window_FindChild(window, nullptr, 0x26);
    g_open->buildRows(list, nullptr, GameAddress<u8 *(__cdecl *)()>(0x004a8380), reinterpret_cast<void(__cdecl *)(u8 *)>((uintptr_t)0x004a8420));
    g_open->columnBottom(list, nullptr, 0, 0);
    g_open->setHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_open->findByName(reinterpret_cast<const char *>((uintptr_t)&gText_Dialog7cc188_74fa84))));
    g_open->fillList(0);
    u8 *const edit = Window_FindChild(window, nullptr, 0x25);
    u8 *const memory = static_cast<u8 *>(g_open->alloc(0x1a4));
    g_open->setImpl(edit, nullptr, memory ? g_open->editPart(memory, nullptr, 0) : nullptr);
    g_open->set4c(edit, nullptr, g_open->font(7));
    g_open->notify40000(edit, nullptr, 0xff, 0xff, 0xff, 0xff);
    g_open->notifyC00(edit, nullptr, 0, 0);
    g_open->setHandler(edit, nullptr, 0x004a77c0);
    g_open->setBuffer(edit, nullptr, reinterpret_cast<uintptr_t>(&gDialog7cc188_7cb8d8), 0x3f);
    g_open->setCaret(edit, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_open->findByName(reinterpret_cast<const char *>((uintptr_t)&gText_Dialog7cc188_74fa84))));
    vcall<void>(edit, 0x24, reinterpret_cast<const void *>(gWindow7cc8c0handler_78a374));
    g_open->post(0x134);
}

void __cdecl Dialog7cc188_FillList(u32 clear) {
    u8 *const strings = g_env->source() + 0x100;
    const char *text = textOf(StdListString_First(strings));
    u8 *const list = Window_FindChild(gDialog7cc188handler_7cc188, nullptr, 0x26);
    if (clear) g_env->clear(list);
    u32 texts[1];
    for (u32 index = 0; *text; ++index) {
        u16 buffer[0x40];
        g_env->swprintf(buffer, reinterpret_cast<const u16 *>((uintptr_t)&gDialog7cc188handler_75442c), text);
        texts[0] = (u32)(uintptr_t)buffer;
        g_env->add(list, nullptr, texts, index, -1, 0);
        text = textOf(StdListString_Next(strings));
    }
    u8 &guard = gGamepause_fb34bf;
    if (!(guard & 1)) {
        guard |= 1;
        Setting_ReadMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (gDialog74fb20_fb3b7c == 1) {
        const u16 *const name = at<const u16 *>(reinterpret_cast<const void *>((uintptr_t)Obj_Get3a4(Online_Instance())), 0x9c);
        if (name && Crt().wcslen(name)) {
            texts[0] = (u32)(uintptr_t)g_env->wideText(0x121);
            g_env->add(list, nullptr, texts, 0xffffffff, -1, 0);
        }
    }
    g_env->refresh(list);
}
