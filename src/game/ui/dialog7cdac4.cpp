#include "dialog7cdac4.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/namedobjects.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../frontend/scenequeue.h"
#include "../render/textparts.h"
#include "dialogbase.h"
#include "focus.h"
#include "hoverlabel.h"
#include "window.h"
#include "window705a20.h"
#include "windowreg.h"

namespace {
const Dialog7cdac4Env kGameEnv = {Window7c24c8_Close, Dialog_Create, Window_SetDepth, TextPart6ff4d0_Ctor, Window_SetImpl, Language_Font, Window_Set4c, Window705a20_SetBuffer, GuiWindows_FindByName, Window705a20_SetCaret, NamedObjects_SetThemeLabelAnsi, Window_Notify3000,
                                  Window_NotifyC00, Language_WideText, HoverLabel_Attach, Class702da0_Push};
const Dialog7cdac4Env *g_env = &kGameEnv;

u8 *&dialog() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cdac4); }

// A child with its part and font 7 (the first thing each of the two gets).
u8 *editChild(u32 id) {
    u8 *const child = Window_FindChild(dialog(), nullptr, id);
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    g_env->setImpl(child, nullptr, block ? g_env->partCtor(block, nullptr, 0) : nullptr);
    g_env->set4c(child, nullptr, g_env->font(7));
    return child;
}

void setCaret(u8 *child) { g_env->setCaret(child, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_env->findByName(reinterpret_cast<const char *>((uintptr_t)0x0074fa84)))); }
}  // namespace

void Dialog7cdac4_SetEnv(const Dialog7cdac4Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog7cdac4_Open() {
    g_env->closeWindow();
    dialog() = g_env->create(reinterpret_cast<u8 *>((uintptr_t)0x007595b0), 0x004c4480, 0);
    g_env->setDepth(dialog(), nullptr, (u16)(*reinterpret_cast<u16 *>(dialog() + 0xe0) + 4));
    u8 *child = editChild(0x11e4);
    g_env->setBuffer(child, nullptr, 0x007cd850, 0xf);
    setCaret(child);
    WString label;
    WString_Ctor(&label);
    u8 *const named = reinterpret_cast<u8 *>((uintptr_t)0x00786b68);
    g_env->setTheme(named, nullptr, reinterpret_cast<const String *>(named), reinterpret_cast<u8 *>(&label));
    vcall<void>(child, 0x24, (u32)(uintptr_t)label.data);
    child = editChild(0x11e3);
    g_env->notify3000(child, nullptr, 1, 0);
    g_env->notifyC00(child, nullptr, 0, 0);
    g_env->setBuffer(child, nullptr, 0x007cd480, 0x7e);
    setCaret(child);
    vcall<void>(child, 0x24, (u32)(uintptr_t)g_env->wideText(0xdc));
    g_env->attachLabel(dialog(), 0x11e5, (u32)(uintptr_t)g_env->wideText(0xda));
    g_env->push(*reinterpret_cast<u8 **>(*reinterpret_cast<u8 **>((uintptr_t)0x00f85614) + 0x14), nullptr, 0x250, 1);
    WString_Dtor(&label);
}
}
