// A tick message sent down the window tree. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the walk calls (the bench points it at a recorder): Window_SendMessage 0x658d9f.
struct WindowTickEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);
};
// The walk's calls go to env (nullptr: the default).
void WindowTick_SetEnv(const WindowTickEnv *env);

// 0x658f97 (cdecl (window); name invented): the window sent 0x1e (0, 0) and counted in 0xfaa60c, then each child (+0x5c, next +0x54) depth first. EAX not kept.
void __cdecl WindowTick_Tick(u8 *window);
// 0x658f6c (cdecl; name invented): the count 0xfaa60c from 0 over the tree of the root 0xfaa5d8, when there is one; the count.
u32 __cdecl WindowTick_TickAll();
}
