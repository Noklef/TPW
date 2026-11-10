// The dialog [0x7cb400]'s handler and end: see dialog7cb400close.h.
#include "dialog7cb400close.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/globalsave.h"
#include "../core/keymap.h"
#include "../core/options.h"
#include "../core/string.h"
#include "../core/vobj2.h"
#include "../files/userslots.h"
#include "../frontend/scenequeue.h"
#include "dialog7cc4b4update.h"
#include "dialogbase.h"
#include "window.h"
#include "dialog74fb20.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const Dialog7cb400CloseEnv kGameEnv = {&Window_SendMessage, &Window_SetFocus, &Window_SetShown, &KeyMaps_EnableAll, &Class702da0_Clear, &Setting_ReadMode, game<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0),
                                       &GlobalSave_Instance, &GlobalSave_Inc20, game<void(__fastcall *)(u8 *)>(0x005c8a10), &Class702da0_Push, &Obj_PickByName, game<void(__fastcall *)(u8 *)>(0x005e2010),
                                       &Dialog7cc4b4_ShowTheme, &Dialog_BaseHandler, game<void(__fastcall *)(u8 *, void *, u32, WString *, u32)>(0x005c7f40), game<void(__fastcall *)(u8 *, void *, u32)>(0x005c83b0)};
const Dialog7cb400CloseEnv *g_env = &kGameEnv;

u16 *text() { return reinterpret_cast<u16 *>((uintptr_t)0x007cb3b8); }
u8 *scenes() { return global<u8 *>(0x00f85614); }

}  // namespace

void Dialog7cb400Close_SetEnv(const Dialog7cb400CloseEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u8 __cdecl Dialog7cb400_Commit(u32 value) {
    if (Crt().wcslen(text()) != 0) {
        while (text()[Crt().wcslen(text()) - 1] == 0x20) {
            text()[Crt().wcslen(text()) - 1] = 0;
            if (Crt().wcslen(text()) == 0) break;
        }
    }
    if (Crt().wcslen(text()) == 0) return 0;
    WString name;
    WString_CtorStr(&name, nullptr, text());
    const u32 chosen = Fn_66a281(reinterpret_cast<const u8 *>(Window_FindChild(global<u8 *>(0x007cb400), nullptr, 0x70c))) == 0x70d;
    g_env->slotsAdd(UserSlots_Instance(), nullptr, value, &name, chosen);
    g_env->slotsChoose(UserSlots_Instance(), nullptr, value);
    WString_Dtor(&name);
    return 1;
}

void __cdecl Dialog7cb400_Close(u32 cancelled) {
    u8 *const window = global<u8 *>(0x007cb3f8);
    g_env->setFocus(reinterpret_cast<u8 *>((uintptr_t)Fn_65d55a(window)), nullptr, 0);
    g_env->send(global<u8 *>(0x007cb3f8), 4, 0, 0);
    g_env->setShown(*reinterpret_cast<u8 **>(global<u8 *>(0x00f82884) + 4), nullptr, 1);
    g_env->setFocus(*reinterpret_cast<u8 **>(global<u8 *>(0x00f82884) + 4), nullptr, 0);
    g_env->enableKeys();
    g_env->clearQueue(*reinterpret_cast<u8 **>(scenes() + 0x14));
    if (cancelled) return;
    if (*reinterpret_cast<s32 *>(UserSlots_Instance() + 0x60) != -1 && global<u32>(0x007cb3fc) != 0) {
        u8 &guard = gDialog7cb400_Close_fb34c0;
        if (!(guard & 1)) {
            guard |= 1;
            g_env->readMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
            g_env->atExit(game<void(__cdecl *)()>(0x005d1750));
        }
        if (gDialog74fb20_fb3b7c == 2) {
            g_env->push(*reinterpret_cast<u8 **>(scenes() + 0x14), nullptr, 0x18a, 1);
        } else {
            g_env->saveCount(g_env->saveInstance());
            g_env->slotsRefresh(UserSlots_Instance());
            g_env->push(*reinterpret_cast<u8 **>(scenes() + 0x14), nullptr, 0x189, 1);
        }
    }
    g_env->pick(*reinterpret_cast<u8 **>(scenes() + 4), nullptr, 1);
    g_env->menuRefresh(*reinterpret_cast<u8 **>(scenes() + 4));
    g_env->showTheme();
}

s32 __cdecl Dialog7cb400_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 0x14:
        global<u32>(0x007cb400) = 0;
        break;
    case 0x100:
        if (a == 0xfffffffe) {
            g_env->send(window, 4, 0, 0);
            return 0;
        }
        if (a == 0xffffffff && Dialog7cb400_Commit(Window_DwordA0(window, nullptr, 0)) == 1) {
            global<u32>(0x007cb3fc) = 1;
            g_env->send(window, 4, 0, 0);
            Dialog7cb400_Close(0);
            return 0;
        }
        break;
    case 0x101:
        if (b == 1) g_env->setShown(Window_FindChild(window, nullptr, 0x70b), nullptr, a == 0x70d ? b : 0);
        break;
    case 0x802:
        g_env->send(window, 0x100, 0xffffffff, 0);
        return 0;
    case 0x804:
        g_env->send(window, 4, 0, 0);
        return 0;
    default:
        break;
    }
    return g_env->base(window, message, a, b);
}
}
