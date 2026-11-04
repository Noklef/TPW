#include "panelkeys.h"

#include <cstdint>

#include "../core/vcall.h"
#include "panel7c2534menus.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const PanelKeysEnv kGameEnv = {&Panel7c2534_Open};
const PanelKeysEnv *g_env = &kGameEnv;

}  // namespace

void PanelKeys_SetEnv(const PanelKeysEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall PanelKeys_HandleMessage(u8 *panel, void *, u32 message, u32 a, u32 b) {
    if (message == 0x1000b && a == 0x1b) {
        u32 taken = 0;
        for (u8 *node = at<u8 *>(at<u8 *>(panel, 0x18), 4); node;) {
            u8 *const object = at<u8 *>(node, 0);
            node = at<u8 *>(node, 4);
            if (vcall<u32>(object, 0x24)) taken |= vcall<u32>(object, 0x18);
        }
        if (!taken) g_env->open(1);
    }
    for (u8 *node = at<u8 *>(at<u8 *>(panel, 0x18), 4); node;) {
        u8 *const object = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        if (vcall<u32>(object, 0x24)) vcall<void>(object, 0x14, message, a, b);
    }
}
}
