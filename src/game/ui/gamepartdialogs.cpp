// The handlers of the dialogs that drive the game object's parts: see gamepartdialogs.h.
#include "gamepartdialogs.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../frontend/scenequeue.h"
#include "dialog7cc4b4update.h"
#include "dialog7cca70.h"
#include "dialogs.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(u8 *p, u32 offset) { return *reinterpret_cast<T *>(p + offset); }

const GamePartDialogEnv kGameEnv = {&Dialog7cca70_Open, &Class702da0_Push, &Dialog7cc4b4_SetEffect, &Dialog7cc4b4_Update};
const GamePartDialogEnv *g_env = &kGameEnv;

u8 *part(u32 offset) { return at<u8 *>(global<u8 *>(0x00f85614), offset); }

}  // namespace

void GamePartDialogs_SetEnv(const GamePartDialogEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Dialog7cc47c_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        global<u32>(0x007cc47c) = 0;
    } else if (message == 0x100) {
        switch (a) {
        case 0x49d5:
            vcall<void>(part(8), 0x2c);
            g_env->open(0x004b8510);
            return 0;
        case 0x49d6:
            vcall<void>(part(8), 0x20);
            vcall<void>(part(4), 0x1c);
            g_env->push(part(0x14), nullptr, 0x258, 1);
            return 0;
        case 0x49d7:
            vcall<void>(part(8), 0x20);
            vcall<void>(part(0xc), 0x1c);
            return 0;
        case 0x49d8:
            vcall<void>(part(8), 0x40);
            break;
        case 0x49db:
            vcall<void>(part(8), 0x3c);
            break;
        case 0x49dc:
            vcall<void>(part(8), 0x38);
            break;
        default:
            break;
        }
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}

extern "C" s32 __cdecl Dialog7cc4b4_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x11) {
        if (a) g_env->update();
        else g_env->setEffect(nullptr, 0);
    } else if (message == 0x14) {
        global<u32>(0x007cc4b4) = 0;
        g_env->setEffect(nullptr, 0);
    } else if (message == 0x100) {
        switch (a) {
        case 0x1e0e8:
            vcall<void>(part(4), 0x40);
            break;
        case 0x1e0e9:
            vcall<void>(part(4), 0x20);
            vcall<void>(part(0xc), 0x1c);
            return 0;
        case 0x1e0f0:
            vcall<void>(part(4), 0x3c);
            break;
        case 0x1e0f1:
            vcall<void>(part(4), 0x38);
            break;
        default:
            break;
        }
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}
