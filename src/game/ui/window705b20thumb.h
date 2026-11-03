// The handler of the scroll bar's thumb (src/game/ui/window705b20: Window705b20_AddThumb sets it). The name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "window705b20.h"

extern "C" {

// What the handler calls (the benches point these at recorders): the ports.
struct Window705b20ThumbEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                                 // Window_SendMessage 0x658d9f
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);                                 // Window_PostMessage 0x658ccc
    u32(__cdecl *defaultProc)(u8 *window, u32 message, u32 a, u32 b);                          // Window_DefaultProc 0x65f6d1
    u16(__fastcall *keepRect)(u8 *window);                                                     // Window_KeepRect 0x65cf3d
    u32(__fastcall *setPos)(u8 *bar, void *, s32 mode, s32 value);                             // Window705b20_SetPos 0x66b088
    void(__fastcall *activate)(u8 *window);                                                    // Window_Activate 0x65dd18
    u16(__fastcall *setFlag139)(u8 *window, void *, u8 flag, u8 notify);                       // Window705a50_SetFlag139 0x668c99
    u16(__fastcall *setState)(u8 *window, void *, u8 state, u8 notify);                        // Window705a50_SetState 0x668bec
    u32(__fastcall *captureMouse)(u8 *window);                                                 // Window_CaptureMouse 0x65e549
    u32(__fastcall *releaseMouse)(u8 *window);                                                 // Window_ReleaseMouse 0x65e56e
    void(__fastcall *notifyScroll)(u8 *bar);                                                   // Window705b20_NotifyScroll 0x66b9bc
    void(__fastcall *notifyScrollEnd)(u8 *bar);                                                // Window705b20_NotifyScrollEnd 0x66b9ef
    u32(__cdecl *moveInside)(u8 *rect, GuiRectByValue bounds, u32 dx, u32 dy, u32 across);     // Window705b20_MoveInside 0x66b771
    u32(__fastcall *setRect)(u8 *window, void *, u8 *rect);                                    // Window_SetRect 0x65cc04
    void(__fastcall *posFromThumb)(u8 *bar);                                                   // Window705b20_PosFromThumb 0x66af19
};
// The handler's calls go to env (nullptr: the defaults).
void Window705b20Thumb_SetEnv(const Window705b20ThumbEnv *env);

// 0x66bb9b (cdecl (window, message, a, b); name invented), for the thumb of a bar (the parent, Fn_65d55a). 0xc: the bar sent it, its answer. 0x13: with [0xfaa5d4], the bar posted 3 (0, 0),
// then the default. 0x15: the rectangle kept, Window705b20_SetPos(-2, 0), then the default. 0x10001 (the mouse in): the mouse object [0x77c48c]'s slot 9 (the bar); with none of the buttons
// +0x11c down, SetFlag139(1, 1). 0x10002 (out): its slot 10; with none down, SetFlag139(0, 1). 0x10003 (moved, a: x << 16 | y): its slot 8 (x signed, y); with button 0 down, the thumb's
// rectangle +0x20 moved inside the bar's +0x2c (Window705b20_MoveInside, by the word differences from button 0's press (sic: whichever button), across by bit 2 of the bar's +0x14c) and set,
// the bar's position read back; the bar's slot 8 called first, its answer dropped. 0x10004 (button a up): SetState(0, 1), the button's bit off +0x11c and its entry 0xfaa598 (12 bytes each)
// not held, the mouse released, SetPos(-2, 0), the rectangle kept, NotifyScrollEnd. 0x10005 (button a down at b): the window activated; unless the entry is held, its place b's halves (x
// signed), held, the bit set, SetFlag139(0, 0), SetState(1, 1), the rectangle kept, the mouse captured, NotifyScroll. All of these 0. 0x1000d: the bar's slot 7 given its slot 8 less a, then
// the default. Anything else: the default (Window_DefaultProc), its EAX.
u32 __cdecl Window705b20Thumb_Proc(u8 *window, u32 message, u32 a, u32 b);
}
