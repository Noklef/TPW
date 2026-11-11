// The handler of the chat rooms' text windows: see roomtexthandler.h.
#include "roomtexthandler.h"

#include <cstdint>
#include <initializer_list>

#include "../ui/window.h"
#include "roomcounts.h"
#include "session.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const RoomTextHandlerEnv kGameEnv = {&Online_Instance, reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32)>(&Window_DwordA0), GameAddress<void(__fastcall *)(u8 *, void *, s32)>(0x005b7910), &RoomCounts_Update,
                                     reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32, u32, u32)>(&Window_CallDefault)};
const RoomTextHandlerEnv *g_env = &kGameEnv;

bool partsReady() {
    for (u32 part : {0xe0u, 0xf8u, 0xfcu, 0xf4u})
        if (!at<u32>(at<u8 *>(g_env->online(), part), 0x28)) return false;
    return true;
}

}  // namespace

void RoomTextHandler_SetEnv(const RoomTextHandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" u32 __cdecl RoomText_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x1e) {
        g_env->update();
    } else if (message == 0x10006 && partsReady()) {
        const s32 room = static_cast<s32>(g_env->dwordA0(window, nullptr, 0)) - 1;
        g_env->enter(g_env->online(), nullptr, room);
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}
