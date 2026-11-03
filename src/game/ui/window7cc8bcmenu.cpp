// The handler of the window [0x7cc8bc]'s name list: see window7cc8bcmenu.h.
#include "window7cc8bcmenu.h"

#include <cstdint>

#include "../world/mousemode.h"
#include "window.h"
#include "window7059f0.h"
#include "window705a50.h"
#include "window7cc8bc.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Window7cc8bcMenuEnv kGameEnv = {Window7059f0_NthItemData, Window7059f0_NthItemCell, OnlineChat_Instance, reinterpret_cast<u8(__fastcall *)(u8 *, void *, u32)>((uintptr_t)0x004ea9a0), reinterpret_cast<void(__cdecl *)(u32, u8 *)>(Window7cc8cc_PlaceAtMouse), Window705a50_SetState, Window_SetDwordA0, Window_CallDefault};
const Window7cc8bcMenuEnv *g_env = &kGameEnv;

}  // namespace

void Window7cc8bc_SetMenuEnv(const Window7cc8bcMenuEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Window7cc8bc_ListHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x402) {
        u8 *const list = Window_FindChild(Window_FindChild(global<u8 *>(0x007cc8bc), nullptr, 0xc3123), nullptr, 0xc3124);
        u32 data, cell;
        g_env->itemData(list, nullptr, (s32)b, &data);
        g_env->itemCell(list, nullptr, (s32)b, 0, &cell);
        if (!g_env->isSelf(g_env->chat(), nullptr, cell)) {
            const bool first = data & 1;
            u8 *const owner = global<u8 *>(first ? 0x007cc8c4 : 0x007cc8c8);
            global<u8 *>(0x007cc8cc) = owner;
            u8 *const placed = Window_FindChild(owner, nullptr, first ? 0x1789 : 0x3003);
            g_env->place(b, placed);
            g_env->setState(Window_FindChild(placed, nullptr, first ? 0x178c : 0x3006), nullptr, (data & 8) ? 1 : 0, 1);
            g_env->setA0(placed, nullptr, 0, cell);
        }
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}
}
