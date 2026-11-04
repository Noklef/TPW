// The button window 0x705a50's handler 0x668f9c and its state's getter 0x668abd. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct Window705a50HandlerEnv {
    void(__fastcall *repeat)(u8 *window);                                  // Window705a50_Repeat
    void(__fastcall *activate)(u8 *window);                                // Window_Activate
    u32(__cdecl *now)();                                                   // GuiClock_Now
    u16(__fastcall *setFlag139)(u8 *window, void *, u8 flag, u8 notify);   // Window705a50_SetFlag139
    u16(__fastcall *setState)(u8 *window, void *, u8 state, u8 notify);    // Window705a50_SetState
    void(__fastcall *startRepeat)(u8 *window);                             // Window705a50_StartRepeat
    u32(__fastcall *stopRepeat)(u8 *window);                               // Window705a50_StopRepeat
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);             // Window_PostMessage
    void(__fastcall *applyCursor)(const u8 *window);                       // Window_ApplyCursor
    u32(__cdecl *defaultProc)(u8 *window, u32 message, u32 a, u32 b);      // Window_DefaultProc
};
void Window705a50Handler_SetEnv(const Window705a50HandlerEnv *env);

extern "C" {

// 0x668abd (thiscall): the state byte +0x138 sign-extended to AX (the rest of EAX the window's address, not kept).
s16 __fastcall Window705a50_StateWord(const u8 *window);

// 0x668f9c (cdecl (window, message, a, b)): 0x1e, +0x11c set: Window705a50_Repeat, then the default. The mouse [0x77c48c]: 0x10001: Window_ApplyCursor, its slot 9 (window); buttons held (+0x11c): those of the three at 0xfaa598 at 2 go to 1,
// the state's bit 2 set, Window_PostMessage (window, 0x10008, the id, a); else the flag +0x139 set. 0x10002: slot 10; held: those at 1 go to 2, the bit 2 clear; else the flag clear. 0x10003: slot 8 (a's high half signed, its low half).
// 0x10005 with a 0 (else the default): Window_Activate, and with the button 0 up: its time (GuiClock_Now, asked twice), place (b), 1; +0x11c's bit 0; the flag clear, the bit 2 set, Window705a50_StartRepeat. 0x10004 with a 0: pressed = the
// button at 1 and Window705a50_StopRepeat's word set; with the look bit 0x10 of +0x48: with the bit 2 the state 0 or 1 by the bit 1 turned over, slot 7; without: the state 0; +0x11c's bit 0 clear, the button's 0 and time cleared, the flag set;
// pressed: Window_PostMessage (the parent, 0x100, the id, the state). Each of these answers 0; the rest go to Window_DefaultProc, its answer returned.
u32 __cdecl Window705a50_Handler(u8 *window, u32 message, u32 a, u32 b);
}
