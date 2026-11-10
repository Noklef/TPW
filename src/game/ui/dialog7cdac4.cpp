#include "dialog7cdac4.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/keymap.h"
#include "../core/namedobjects.h"
#include "../core/options.h"
#include "../core/string.h"
#include "../core/vcall.h"
#include "../frontend/scenequeue.h"
#include "../render/textparts.h"
#include "dialogbase.h"
#include "focus.h"
#include "hoverlabel.h"
#include "../online/session.h"
#include "window.h"
#include "window705a20.h"
#include "windowreg.h"
#include "dialog74fb20.h"

namespace {
const Dialog7cdac4Env kGameEnv = {Window7c24c8_Close, Dialog_Create, Window_SetDepth, TextPart6ff4d0_Ctor, Window_SetImpl, Language_Font, Window_Set4c, Window705a20_SetBuffer, GuiWindows_FindByName, Window705a20_SetCaret, NamedObjects_SetThemeLabelAnsi, Window_Notify3000,
                                  Window_NotifyC00, Language_WideText, HoverLabel_Attach, Class702da0_Push};
const Dialog7cdac4Env *g_env = &kGameEnv;

u8 *&dialog() { return gDialog7cdac4_7cdac4; }

// A child with its part and font 7 (the first thing each of the two gets).
u8 *editChild(u32 id) {
    u8 *const child = Window_FindChild(dialog(), nullptr, id);
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    g_env->setImpl(child, nullptr, block ? g_env->partCtor(block, nullptr, 0) : nullptr);
    g_env->set4c(child, nullptr, g_env->font(7));
    return child;
}

void setCaret(u8 *child) { g_env->setCaret(child, nullptr, reinterpret_cast<u8 *>((uintptr_t)g_env->findByName(reinterpret_cast<const char *>((uintptr_t)0x0074fa84)))); }

const Dialog7cdac4HandlerEnv kGameHandlerEnv = {Window_SendMessage, Window_PostMessage, KeyMaps_EnableAllAndClearFlag, KeyMaps_ResetAllAndSetFlag, Focus_Restore, Dialog_BaseHandler, Setting_ReadMode,
                                                reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), Online_Instance,
                                                reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32, u32, u32, u32, u32, u32)>((uintptr_t)0x005b7ba0), Class702da0_Clear, Window_SetFocus};
const Dialog7cdac4HandlerEnv *g_handler = &kGameHandlerEnv;

u8 *pointerAt(u32 address, u32 offset) { return *reinterpret_cast<u8 **>(*reinterpret_cast<u8 **>((uintptr_t)address) + offset); }
u32 wordAt(u32 address, u32 offset) { return *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>((uintptr_t)address) + offset); }
}  // namespace

void Dialog7cdac4_SetEnv(const Dialog7cdac4Env *env) { g_env = env ? env : &kGameEnv; }
void Dialog7cdac4_SetHandlerEnv(const Dialog7cdac4HandlerEnv *env) { g_handler = env ? env : &kGameHandlerEnv; }

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

s32 __cdecl Dialog7cdac4_Handler(u8 *window, u32 message, u32 a, u32 b) {
    const Dialog7cdac4HandlerEnv *const env = g_handler;
    switch (message) {
    case 0x14:
        dialog() = nullptr;
        env->keysEnable();
        env->send(pointerAt(0x00f82884, 4), 6, 1, 0);
        env->focusRestore();
        break;
    case 0x15:
        env->send(pointerAt(0x00f82884, 4), 6, 0, 0);
        env->keysReset();
        break;
    case 0x100:
        if (a == 0xfffffffe || a == 0xffffffff) {
            if (a == 0xffffffff) {
                u8 &guard = gDialog7cdac4_Handler_fb34bb;
                const u8 bits = guard;
                if (!(bits & 1)) {
                    guard = bits | 1;
                    env->readMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
                    env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
                }
                const u32 modeTwo = gDialog74fb20_fb3b7c == 2;
                const u32 first = *reinterpret_cast<u32 *>((uintptr_t)0x00f82840), second = wordAt(0x00f82888, 8);
                env->chatSend(env->chat(), nullptr, first, second, 0x007cd850, 0x007cd480, modeTwo, 0x004c4470, 0);
            }
            env->clearQueue(pointerAt(0x00f85614, 0x14));
            env->send(dialog(), 4, 0, 0);
            return 0;
        }
        break;
    case 0x802:
        if (a == 0x11e4) env->setFocus(Window_FindChild(window, nullptr, 0x11e3), nullptr, 0);
        break;
    case 0x803:
        env->setFocus(Window_FindChild(window, nullptr, a == 0x11e4 ? 0x11e3 : 0x11e4), nullptr, 0);
        break;
    case 0x804:
        env->post(window, 4, 0, 0);
        break;
    default:
        break;
    }
    return env->baseHandler(window, message, a, b);
}
}
