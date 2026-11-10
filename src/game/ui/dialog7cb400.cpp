#include "dialog7cb400.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../frontend/scenequeue.h"
#include "../render/textparts.h"
#include "dialogbase.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "radiogroup.h"
#include "window.h"
#include "window705a20.h"
#include "windowreg.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7cb400Env kGameEnv = {&Class702da0_Push, &Dialog_Create, &Window705a20_SetBuffer, &TextPart6ff4d0_Ctor, &TextPart_Ctor, &TextPart6ff530_Ctor, &Window_SetImpl, &Window_Set4c, &Window_Notify40000, &Window_Notify3000, &Window_NotifyC00,
                                  &Window_NotifyImpl100, &GuiWindows_FindByName, &Window705a20_SetCaret, &Language_WideText, &Window_SetFocus, &Window705a20_SelectAll, &RadioGroup_Select, &HoverLabel_SetYellow};
const Dialog7cb400Env *g_env = &kGameEnv;

void slot9Text(u8 *window, u32 text) { vcall<void>(window, 0x24, (u32)(uintptr_t)g_env->wideText(text)); }

void choice(u8 *window, u32 text) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    g_env->setImpl(window, nullptr, memory ? g_env->partText(memory, nullptr, 0) : nullptr);
    g_env->notifyC00(window, nullptr, 1, 1);
    g_env->set4c(window, nullptr, Language_Font(6));
    g_env->notify40000(window, nullptr, 0, 0, 0, 0xff);
    g_env->notifyImpl100(window, nullptr, 0);
    slot9Text(window, text);
}
}  // namespace

void Dialog7cb400_SetEnv(const Dialog7cb400Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog7cb400_Open(u32 value) {
    u8 rect[0x10];
    GuiRect_Ctor(rect);
    if (global<u32>(0x007cb3f8)) {
        g_env->push(*reinterpret_cast<u8 **>(global<u8 *>(0x00f85614) + 0x14), nullptr, 0x23a, 1);
        u8 *const dialog = g_env->create(gDialog7cb400_Open_753f50, 0x004a6d00, 1);
        global<u8 *>(0x007cb400) = dialog;
        Window_GetSavedRect(dialog, nullptr, rect);
        u8 *const edit = Window_FindChild(dialog, nullptr, 0x70a);
        g_env->setBuffer(edit, nullptr, 0x007cb3b8, 0x10);
        u8 *memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
        g_env->setImpl(edit, nullptr, memory ? g_env->partEdit(memory, nullptr, 0) : nullptr);
        g_env->set4c(edit, nullptr, Language_Font(6));
        g_env->notify40000(edit, nullptr, 0xff, 0xff, 0xff, 0xff);
        g_env->notify3000(edit, nullptr, 0, 0);
        g_env->setCaret(edit, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_env->findByName(reinterpret_cast<const char *>(0x0074fa84))));
        Fn_66781a(edit, nullptr, 0x00752588);
        slot9Text(edit, 0xf3);
        g_env->setFocus(edit, nullptr, 0);
        g_env->selectAll(edit);
        Window_SetDwordA0(dialog, nullptr, 0, value);
        u8 *const group = Window_FindChild(global<u8 *>(0x007cb400), nullptr, 0x70c);
        choice(Window_FindChild(group, nullptr, 0x70d), 0xf1);
        choice(Window_FindChild(group, nullptr, 0x70e), 0xf2);
        g_env->selectRadio(group, nullptr, 0x7a17);
        memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
        u8 *const part = memory ? g_env->partTitle(memory, nullptr, 0) : nullptr;
        g_env->setImpl(Window_FindChild(dialog, nullptr, 0x709), nullptr, part);
        u8 *const font = Language_Font(5);
        g_env->set4c(Window_FindChild(dialog, nullptr, 0x709), nullptr, font);
        g_env->notifyC00(Window_FindChild(dialog, nullptr, 0x709), nullptr, 1, 1);
        slot9Text(Window_FindChild(dialog, nullptr, 0x709), 0xef);
        g_env->yellow(Window_FindChild(dialog, nullptr, 0x709));
    }
    GuiRect_Dtor(rect);
}
}
