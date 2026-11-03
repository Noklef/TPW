// The edit window 0x705a20's handler 0x66857d and its mouse: reimplementation of 0x66857d, 0x6680b7, 0x668269 and 0x6681ac. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct Window705a20HandlerEnv {
    void(__fastcall *moveCaret)(u8 *window, void *, u32 a, u32 b);                 // Window705a20_MoveCaret
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                          // Window_SetFocus
    u32(__fastcall *timerStop)(u8 *node);                                          // Fn_662097
    u32(__fastcall *timerStart)(u8 *node);                                         // WindowTimer_Start
    void(__fastcall *insertMode)(u8 *window, void *, u32 mode);                    // Window705a20_SetInsertMode
    void(__fastcall *selectAll)(u8 *window);                                       // Window705a20_SelectAll
    void(__fastcall *toggleInsert)(u8 *window);                                    // Window705a20_ToggleInsert
    void(__fastcall *keyDown)(u8 *window, void *, u32 key, u32 b);                 // Window705a20_KeyDown
    u32(__fastcall *keyToParent)(u8 *window, void *, u32 key, u32 value);          // Window705a20_KeyToParent
    void(__fastcall *typeChar)(u8 *window, void *, u32 ch);                        // Window705a20_TypeChar
    void(__fastcall *postIfHit)(u8 *window, void *, u32 message, u32 button, u32 point);   // Window_PostIfHit
    u32(__cdecl *defaultProc)(u8 *window, u32 message, u32 a, u32 b);              // Window_DefaultProc
    void(__fastcall *setSelection)(u8 *window, void *, u32 cursor, u32 mark, u32 anchor);  // Window705a20_SetSelection
};
void Window705a20Handler_SetEnv(const Window705a20HandlerEnv *env);

extern "C" {

// 0x6681ac (cdecl (list, x, y, last)): -1 with no list. Of the places 0..last (signed; slot 4 of the list (1, i), the point's words +4, +6) the first with the least |x - +4| + (|y - +6| << 14) (unsigned), -1 for none.
s32 __cdecl Window705a20_NearestPlace(u8 *list, s32 x, s32 y, s32 last);

// 0x6680b7 (thiscall (button, point: x high, y low), pops 8): nothing with the byte +0x161 set, another button, or no +0x4c, buffer +0x138 or list +0x9c, or when the list's slot 4 (1, 0) answers 0. The place nearest the
// point inside the window (x less the word +0x30; y's low word unsigned less the word +0x32, plus half the height, slot 1 of +0x4c) over the buffer's wcslen: found, Window705a20_SetSelection (it, 1, -1).
void __fastcall Window705a20_MouseDown(u8 *window, void *, u32 button, u32 point);

// 0x668269 (thiscall (point), pops 4): as 0x6680b7, for a window with the bit 0 of +0x11c held, the bytes +0x161 and +0x162 clear and a buffer and a list: Window705a20_SetSelection (-1, 1, it).
void __fastcall Window705a20_MouseMove(u8 *window, void *, u32 point);

// 0x66857d (cdecl (window, message, a, b)): 0x13 Window705a20_MoveCaret (the words a, b). 0x1a Window_SetFocus (1). 0x20: the timer +0x164's Fn_662097 (when set), insert mode 0. 0x1f: WindowTimer_Start, insert mode 1, a 0: Window705a20_SelectAll.
// 0x10 ToggleInsert. 0x10003 MouseMove (a). 0x10005 MouseDown (a, b). 0x10006 and 0x10007, with Fn_668800's byte clear, Window_PostIfHit and then Window_SetFocus (1) and SelectAll. 0x1000a KeyDown, 0x1000b KeyToParent (the word a, b); 0x1000c TypeChar
// (the word a). 0x10004 the do-nothing stub 0x66825c. All then go to Window_DefaultProc, its answer returned.
u32 __cdecl Window705a20_Handler(u8 *window, u32 message, u32 a, u32 b);
}
