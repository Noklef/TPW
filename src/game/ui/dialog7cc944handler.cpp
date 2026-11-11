// The handler of the dialog [0x7cc944]: see dialog7cc944handler.h.
#include "dialog7cc944handler.h"

#include <cstdint>

#include "../core/language.h"
#include "../frontend/scenequeue.h"
#include "../online/session.h"
#include "dialog7cc984open.h"
#include "window.h"
#include "window7059f0.h"
#include "window705a20.h"
#include "window705a50.h"
#include "window705a50handler.h"
#include "window7cc8c0handler.h"
#include "dialog7cc944.h"
#include "../core/imports.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Dialog7cc944HandlerEnv kGameEnv = {&Window_SetFocus, &Class702da0_Clear, &Window_SendMessage, &Window_PostMessage, GameAddress<void(__fastcall *)(u8 *, void *, u32, u32, u32)>(0x005bcbc0), &Window7059f0_Clear, &Language_WideText,
                                         &Window7059f0_AddItem, &Window7059f0_NthItemData, &Dialog7cc984_Open, &Window705a50_StateWord, &Window705a50_SetState, &Window705a20_SelectAll, &Window_CallDefault};
const Dialog7cc944HandlerEnv *g_env = &kGameEnv;

void close() {
    g_env->clearScenes(at<u8 *>(global<u8 *>(0x00f85614), 0x14));
    g_env->send(gDialog7cc944_7cc944, 4, 0, 0);
    if (void(__cdecl *const done)() = global<void(__cdecl *)()>(reinterpret_cast<uintptr_t>(&gDialog7cc944_7cc948))) done();
}

void search() {
    g_env->search(at<u8 *>(Online_Instance(), 0xec), nullptr, reinterpret_cast<uintptr_t>(&gDialog7cc944_7cc908), 0x004bf9d0, 0);
    if (u8 *const dialog = gDialog7cc944_7cc944) {
        u8 *const list = Window_FindChild(dialog, nullptr, 0xe05b);
        g_env->clearList(list);
        const u32 texts[6] = {static_cast<u32>(reinterpret_cast<uintptr_t>(g_env->text(0x134))), reinterpret_cast<uintptr_t>(&gDialog7cc944handler_759158), reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_78a374), 0, 0, 0};
        g_env->addItem(list, nullptr, texts, 0xffffffff, -1, 1);
    }
}

}  // namespace

void Dialog7cc944Handler_SetEnv(const Dialog7cc944HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Dialog7cc944_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
        case 0x14:
            gDialog7cc944_7cc944 = nullptr;
            g_env->focus(at<u8 *>(global<u8 *>(0x00f82884), 4), nullptr, 0);
            break;
        case 0x1f:
            g_env->focus(Window_FindChild(window, nullptr, 0xe05a), nullptr, 0);
            break;
        case 0x100:
            if (a == 0xfffffffe) {
                close();
                return 0;
            }
            if (a == 0xe059) {
                if (b == 1)
                    search();
                else
                    (*reinterpret_cast<void(__stdcall *const *)(u32)>((uintptr_t)IMPORT(Kernel32_OutputDebugStringA)))(reinterpret_cast<uintptr_t>(&gDialog7cc944_Handler_75910c));   // OutputDebugStringA
            }
            break;
        case 0x400:
            if (u8 *const child = Window_FindChild(window, nullptr, a)) {
                if (b != 0xffffffff) {
                    u32 data = b;
                    g_env->nth(child, nullptr, static_cast<s32>(b), &data);
                    if (data && data != 0xffffffff) g_env->open(reinterpret_cast<const u8 *>((uintptr_t)data), window, 0, 0x004c0830, 0);
                    return 0;
                }
            }
            break;
        case 0x802:
            if (!g_env->stateWord(Window_FindChild(window, nullptr, 0xe059))) {
                g_env->post(window, 0x100, 0xe059, 1);
                g_env->setState(Window_FindChild(window, nullptr, 0xe059), nullptr, 1, 1);
                g_env->selectAll(Window_FindChild(window, nullptr, a));
            }
            break;
        case 0x1000b:
            if (a == 0x1b) {
                close();
                return 0;
            }
            break;
    }
    return g_env->handler(window, nullptr, message, a, b);
}
