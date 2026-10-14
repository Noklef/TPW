// The application globals and message pump: reimplementation of 0x5f86a0-0x5f8785.
//
// The state sits at 0xfa7608: the quit request (set by the screen window's WM_CLOSE), the
// instance, the main window, a window procedure the screen window hands its messages to, the
// icon resource id and whether the application is active (WM_ACTIVATEAPP). Nothing in the game
// sets the window procedure or the icon id.
#pragma once

#include <cstddef>

#include "../../../re/types.h"

struct AppState {          // 0xfa7608
    s32 quit;
    void *instance;        // +4
    void *window;          // +8
    s32(__stdcall *wndProc)(void *window, u32 message, u32 wParam, s32 lParam);  // +0xc
    u16 iconId;            // +0x10
    u16 unknown12;
    u8 active;             // +0x14
};
static_assert(offsetof(AppState, active) == 0x14, "AppState::active is at +0x14");

extern "C" {
void *__cdecl App_GetInstance();             // 0x5f86a0
void __cdecl App_SetInstance(void *i);       // 0x5f86b0
void *__cdecl App_GetWindow();               // 0x5f86c0
void __cdecl App_SetWindow(void *w);         // 0x5f86d0
// 0x5f86e0: PeekMessage (PM_REMOVE), TranslateMessage and DispatchMessage for at most 2000
// messages; the last PeekMessage result (non-zero when it stopped at the limit).
s32 __cdecl App_PumpMessages();
void __cdecl App_SetQuit(s32 quit);          // 0x5f8750
// 0x5f8760: the window procedure the screen window hands its messages to (0: DefWindowProc).
s32(__stdcall *__cdecl App_GetWndProc())(void *, u32, u32, s32);
u16 __cdecl App_GetIconId();                 // 0x5f8770: 0 for none
const char *__cdecl App_GetName();           // 0x5f8780: "Bullfrog" (0x775708)
}

struct AppEnv {
    AppState *state;  // 0xfa7608
    const char *name; // 0x775708
    s32(__stdcall *const *peekMessage)(void *msg, void *window, u32 first, u32 last, u32 remove);
    s32(__stdcall *const *translateMessage)(const void *msg);
    s32(__stdcall *const *dispatchMessage)(const void *msg);
};
// The calls and state go through env (nullptr: the game's).
void App_SetEnv(const AppEnv *env);
