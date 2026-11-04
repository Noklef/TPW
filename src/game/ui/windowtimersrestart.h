// The window timers set going again from now (windowtimers.h has the list). Reimplementation of 0x66213e and 0x662420. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What they call (the port by default; the bench's recorder: its own bench has it).
struct WindowTimersRestartEnv {
    u32(__cdecl *now)();   // GuiClock_Now
};
void WindowTimersRestart_SetEnv(const WindowTimersRestartEnv *env);

extern "C" {

// 0x66213e (thiscall): the timer's +4 the clock's now.
void __fastcall WindowTimer_Restart(u8 *timer);

// 0x662420 (cdecl): with the flag [0xfaa638] set: the flag 0 and every timer of the list [0xfaa634] (next +0x18) restarted.
void __cdecl WindowTimers_RestartAll();
}
