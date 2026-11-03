// The view controls dialog's handler: see viewbuttons.h.
#include "viewbuttons.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../frontend/scenequeue.h"
#include "../frontend/viewcontrols.h"
#include "../online/session.h"
#include "../world/object.h"
#include "dialog7cc944.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const ViewButtonsEnv kGameEnv = {&Window_CallDefault, &ViewCtl_UseSecondVector, &ViewCtl_UseFirstVector, &ViewCtl_Lower48, &ViewCtl_Raise48, &ViewCtl_Raise50, &ViewCtl_Lower50, &Online_Instance, &Obj_HasActivity, &Dialog7cc944_Open, &Class702da0_Push};
const ViewButtonsEnv *g_env = &kGameEnv;

u8 *game(u32 offset) { return at<u8 *>(global<u8 *>(0x00f85614), offset); }

bool chatReady() {
    if (g_env->hasActivity(g_env->online())) return false;
    return at<u32>(at<u8 *>(g_env->online(), 0xe0), 0x28) && at<u32>(at<u8 *>(g_env->online(), 0xf8), 0x28) && at<u32>(at<u8 *>(g_env->online(), 0xfc), 0x28);
}

}  // namespace

void ViewButtons_SetEnv(const ViewButtonsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl ViewButtons_Handler(u8 *window, u32 message, u32 a, u32 b) {
    const u32 object = Window_DwordA0(window, nullptr, 0) ? 0x10 : 0xc;
    if (message == 0x14) {
        global<u32>(0x007cdb0c) = 0;
    } else if (message == 0x100) {
        if (a == 0x16000) {
            vcall<void>(game(0xc), 0x20);
            vcall<void>(game(4), 0x1c);
            g_env->push(game(0x14), nullptr, 0x258, 1);
            return 0;
        }
        if (a == 0x16003) {
            if (chatReady()) {
                vcall<void>(game(0xc), 0x48);
                g_env->openChat(0x004c4fa0);
            }
        } else if (a == 0x16006) {
            (b == 1 ? g_env->useSecond : g_env->useFirst)(game(object));
        }
    } else if (message == 0x10008 || message == 0x10009) {
        if (a == 0x16001) g_env->lower50(game(object));
        else if (a == 0x16002) g_env->lower48(game(object));
        else if (a == 0x16004) g_env->raise48(game(object));
        else if (a == 0x16005) g_env->raise50(game(object));
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}
