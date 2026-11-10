#include "window7cc8bchandler.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../online/chatcommand.h"
#include "../world/mousemode.h"
#include "gamepause.h"
#include "window.h"
#include "window7059f0.h"
#include "window7cc8bc.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Window7cc8bcHandlerEnv kGameEnv = {&OnlineChat_Instance,      &GamePause_Hide,          &ChatAvatar_Vote,    &ChatAvatar_EarmuffsOn, &ChatAvatar_EarmuffsOff,
                                         &ChatAvatar_FillRoomList,  &ChatAvatar_FillBuddyList, &Window7059f0_Clear, &Window_CallDefault};
const Window7cc8bcHandlerEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void Window7cc8bcHandler_SetEnv(const Window7cc8bcHandlerEnv *env) { g_env = env ? env : &kGameEnv; }

s32 __cdecl Window7cc8bc_Handler(u8 *window, u32 message, u32 a, u32 b) {
    const Window7cc8bcHandlerEnv *const E = g_env;
    if (message == 0x101) {
        const bool rooms = a == 0xc3126;
        gWindow7cc8bc_7cc8d0 = rooms ? 0 : 1;
        E->clear(Window_FindChild(Window_FindChild(gWindow7cc8bc_7cc8bc, nullptr, 0xc3123), nullptr, 0xc3124));
        if (rooms)
            E->fillRooms(E->chat());
        else
            E->fillBuddies(E->chat());
    } else if (message == 0x100) {
        if (a == 0xc3128) {
            if (b == 1)
                E->earmuffsOn(E->chat());
            else
                E->earmuffsOff(E->chat());
        } else if (a == 0xc3129) {
            E->vote(E->chat());
            vcall<void>(Window_FindChild(window, nullptr, 0xc3129), 0x14, 0);
        } else if (a == 0xc312a) {
            E->pauseHide();
        }
    }
    return E->callDefault(window, nullptr, message, a, b);
}
}
