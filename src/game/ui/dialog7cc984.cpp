// The dialog [0x7cc984]: see dialog7cc984.h.
#include "dialog7cc984.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/vcall.h"
#include "../frontend/scenequeue.h"
#include "../online/session.h"
#include "focus.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Dialog7cc984Env kGameEnv = {Window_SendMessage, Class702da0_Clear, Online_Instance, Online_UnpublishPark, reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32, u32, u32)>((uintptr_t)0x005b86a0), Window_SetFocus, Focus_Restore};
const Dialog7cc984Env *g_env = &kGameEnv;

u8 *parentOf(u8 *window) { return reinterpret_cast<u8 *>((uintptr_t)Fn_65d55a(window)); }
u8 *ownerWindow() { return at<u8 *>(global<u8 *>(0x00f82884), 4); }
u8 *gamePart(u32 offset) { return at<u8 *>(global<u8 *>(0x00f85614), offset); }

void closeDialog() {
    g_env->clear(gamePart(0x14));
    g_env->send(gDialog7cc984_7cc984, 4, 0, 0);
}

s32 cancel(u8 *window) {
    if (parentOf(window) == global<u8 *>(0x00faa5d8)) g_env->send(ownerWindow(), 6, 1, 0);
    closeDialog();
    if (const auto callback = global<void(__cdecl *)()>(0x007cca48)) callback();
    return 0;
}

void closeFromParent(u8 *window) {
    if (parentOf(window) != ownerWindow()) g_env->send(parentOf(window), 4, 0, 0);
    else closeDialog();
}

}  // namespace

void Dialog7cc984_SetEnv(const Dialog7cc984Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Dialog7cc984_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        gDialog7cc984_7cc984 = nullptr;
        if (parentOf(window) && !at<u8>(parentOf(window), 4)) g_env->setFocus(parentOf(window), nullptr, 0);
        else g_env->restoreFocus();
    } else if (message == 0x100) {
        if (a == 0xfffffffe) return cancel(window);
        if (a == 0xe592e) {
            closeFromParent(window);
            g_env->request(g_env->online(), nullptr, 0x007cc988, 0x004c07d0, 0);
            return 0;
        }
        if (a == 0xe592f) {
            const u32 park = global<u32>(0x007cc990);
            g_env->unpublish(g_env->online(), nullptr, park);
            closeFromParent(window);
            g_env->send(ownerWindow(), 6, 1, 0);
            if (vcall<u32>(gamePart(8), 0x24)) vcall<void>(gamePart(8), 0x28);
            else vcall<void>(gamePart(0xc), 0x44);
            return 0;
        }
    } else if (message == 0x1000b && a == 0x1b) {
        return cancel(window);
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}
