// The application globals and message pump: see app.h.
#include "app.h"

#include <cstdint>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const AppEnv kGameEnv = {
    at<AppState *>(0x00fa7608),
    at<const char *>(0x00775708),
    at<s32(__stdcall *const *)(void *, void *, u32, u32, u32)>(0x006fd354),
    at<s32(__stdcall *const *)(const void *)>(0x006fd358),
    at<s32(__stdcall *const *)(const void *)>(0x006fd35c),
};
const AppEnv *g_env = &kGameEnv;

}  // namespace

void App_SetEnv(const AppEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void *__cdecl App_GetInstance() { return g_env->state->instance; }
void __cdecl App_SetInstance(void *i) { g_env->state->instance = i; }
void *__cdecl App_GetWindow() { return g_env->state->window; }
void __cdecl App_SetWindow(void *w) { g_env->state->window = w; }

s32 __cdecl App_PumpMessages() {
    u32 msg[7];  // MSG
    s32 more = (*g_env->peekMessage)(msg, nullptr, 0, 0, 1);
    for (u32 left = 2000; more && left; --left) {
        (*g_env->translateMessage)(msg);
        (*g_env->dispatchMessage)(msg);
        more = (*g_env->peekMessage)(msg, nullptr, 0, 0, 1);
    }
    return more;
}

void __cdecl App_SetQuit(s32 quit) { g_env->state->quit = quit; }
s32(__stdcall *__cdecl App_GetWndProc())(void *, u32, u32, s32) { return g_env->state->wndProc; }
u16 __cdecl App_GetIconId() { return g_env->state->iconId; }
const char *__cdecl App_GetName() { return g_env->name; }

}  // extern "C"
