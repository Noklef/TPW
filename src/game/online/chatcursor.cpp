#include "chatcursor.h"

#include <cstdint>

#include "../core/thunks.h"
#include "../world/mapclick.h"
#include "../world/mousemode.h"
#include "chatavatar.h"

namespace {

const ChatCursorEnv kPorts = {&Wrap_KeyMap_AsyncModifiers_4815c0, &GameMode_MapClick, &OnlineChat_Instance, &ChatAvatar_GoToCell, &ChatAvatar_MoveToCell};
const ChatCursorEnv *g_env = &kPorts;

u32 CursorId() { return *reinterpret_cast<const u16 *>((uintptr_t)0x007b05cc); }

u32 CursorDword(void *ecx) { return (static_cast<u32>(reinterpret_cast<uintptr_t>(ecx)) & 0xffff0000) | CursorId(); }

}  // namespace

void ChatCursor_SetEnv(const ChatCursorEnv *env) { g_env = env ? env : &kPorts; }

extern "C" {

void __fastcall ChatCursor_Click(void *, void *, u32, u32) {
    const u32 id = CursorId();
    const u32 modifiers = g_env->modifiers();
    g_env->click(static_cast<s32>((id - 1) & 0x7f), static_cast<s32>((id - 1) >> 7), modifiers, 1);
}

u32 __fastcall ChatCursor_GoTo(void *ecx, void *, u32, u32) {
    const u32 cell = CursorDword(ecx);
    return g_env->goTo(g_env->chat(), nullptr, cell);
}

u32 __fastcall ChatCursor_MoveTo(void *ecx, void *, u32, u32) {
    const u32 cell = CursorDword(ecx);
    return g_env->moveTo(g_env->chat(), nullptr, cell);
}
}
