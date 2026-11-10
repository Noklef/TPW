// The handler of the online view's windows [0x7cc8bc] and [0x7cc8c0]. Reimplementation of 0x4bd4a0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; Mouse_HandleButton 0x46c210 and agent B's and this range's unported 0x4a9380 / 0x48c830 by address; the bench's recorders).
struct Window7cc8c0HandlerEnv {
    u16(__fastcall *getRect)(u8 *window, void *, u8 *rect, u32 inner);                     // Window_GetRect
    u32(__cdecl *place)(u32 x, u32 y);                                                     // HelpBg_Place
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                             // Window_SendMessage
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);                             // Window_PostMessage
    u32(__fastcall *capture)(u8 *window);                                                  // Window_CaptureMouse
    u32(__fastcall *release)(u8 *window);                                                  // Window_ReleaseMouse
    void(__cdecl *cameraInput)(u32 message, u32 a, u32 b);                                 // Camera_Input (it reads the first two)
    u8 *(__fastcall *chatCtor)(u8 *chat);                                                  // OnlineChat_Ctor
    u32(__fastcall *say)(u8 *avatar);                                                      // ChatAvatar_SayPosition
    u8 *(__cdecl *chat)();                                                                 // OnlineChat_Instance
    void(__fastcall *moveTo)(u8 *avatar, void *, u32 x, u32 y);                            // ChatAvatar_MoveTo
    void(__fastcall *moveTo3)(u8 *avatar, void *, u32 x, u32 y);                           // ChatAvatar_MoveTo3
    u16 *(__cdecl *objectAt)(u16 *out, s32 x, s32 y);                                      // Map_ObjectOrEntranceAt
    u16 *(__fastcall *defWord)(const u8 *object, void *, u16 *out);                        // Object_DefWord284
    u8 *(__fastcall *def)(const u8 *object);                                               // Object_GetDef
    void(__fastcall *follow)(const u8 *object, void *, u32 arg);                           // Object_FollowCamera
    void(__cdecl *setText)(const u16 *text);                                               // HelpBg_SetText
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);                // Charset_FormatWide
    u8 *(__cdecl *name)(u8 *thing);                                                        // Thing_NameBuffer7ba058
    void(__cdecl *highlight)();                                                            // Highlight_Update
    void(__cdecl *unhighlight)();                                                          // Highlight_Clear
    u32(__cdecl *modifiers)();                                                             // KeyMap_Modifiers
    u32(__fastcall *fireDown)(u8 *table, void *, u32 a, u32 b);                            // Actions_FireDown
    u32(__fastcall *fireUp)(u8 *table, void *, u32 a, u32 b);                              // Actions_FireUp
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                                  // Window_SetFocus
    u32(__fastcall *killFocus)(u8 *window, void *, u32 a);                                 // Window_KillFocus
    s32(__fastcall *findKey)(const u8 *map, void *, u32 key, u32 modifiers);               // KeyMap_FindKey
    void(__cdecl *button)(u32 button, u32 down, s32 y, s32 x, u32 zero);                   // Mouse_HandleButton 0x46c210
    void(__cdecl *escape)();                                                               // 0x4a9380
    void(__cdecl *leave)(u32 arg);                                                         // 0x48c830
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);           // Window_CallDefault
};
void Window7cc8c0Handler_SetEnv(const Window7cc8c0HandlerEnv *env);

extern "C" {

// 0x4bd4a0 (cdecl (window, message, a, b); handler of [0x7cc8bc] and [0x7cc8c0]). Before the camera: 0x11 with a == 1: the window's rectangle got (a) and, on [0x7cc8bc], the
// byte [0x7cc8b4] 1, any other a on [0x7cc8bc] 0; 0x14: on [0x7cc8bc] it cleared, HelpBg_Place(-1, -1), [0x7cc8c0], [0x7cc8c8], [0x7cc8c4] told 4; on [0x7cc8c0] it cleared;
// 0x10005 / 0x10004 with a == 1 the mouse captured / released. Then Camera_Input(message, a, b), and:
// 0x1e: on [0x7cc8c0] the chat's position followed (the flag [0x790ab0] & 0x400's rising edge: chat made and the position said; its falling edge with & 0x16: the byte
// [0x7cc8d4] 1 and the timer [0x7cc8d8] 0; the timer run down; on the byte the camera's position packed and, when changed or on that edge, the avatar moved), and with the
// window's Fn_65d57c bit 0 the help text: the state [0x7cc8e4] 0 when [0x7cc8dc] and [0x7cc8e0] are clear, 2 over an object of kind 3 whose definition's word 0x284 is 0 and whose
// +0x4ac is 0 (or 3 with its +0x32 bit 4; [0x7cc8e8] the object, highlighted), then with mode 9 3 or 4, else 1 for 0; a changed state sets the text and cursor.
// 0x10001 / 0x10002: [0x7cc8e0] ([0x7cc8c0]) or [0x7cc8dc] set / clear (0x10001 the state 0 too). 0x10004: Actions_FireUp(a - 0x10, modifiers). 0x10005: Actions_FireDown the
// same; button a == 0 first follows the state's object or focuses [0x7cc8bc]; else the holder made and Mouse_HandleButton(0xf0 + (a != 0), 1, y, x, 0), x and y from b by the
// scales [0xfaa5c0] / 1536 and [0xfaa5c4] / 2048. 0x10007 with a == 0: the holder made and Global7b05d8_CallSlot14(y, x). 0x1000a: Actions_FireDown(a, b's byte 2). 0x1000b, b's
// byte 2 the modifiers: 9, 13, 0x2f with [0x7cc8b4] and none: focus to the edit box 0xc312c (0x2f posted 0x1000c too); 0x73 with none and [0x7cc6a0] clear: the two windows
// swapped by 6 and focused; any other: KeyMap_FindKey's action 0 (mode 9: a new CMMOnline, else 0x48c830(0)), 0xf (0x4a9380), 0x10 (a new 0x6fead0 mode, the cursor 0x13); other
// Actions_FireUp(a, modifiers). Always answers Window_CallDefault's.
s32 __cdecl Window7cc8c0_Handler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[108], gWindow7cc8c0handler_78a374, 0x0078a374, 0x6c);
GAME_DATA(u8, gWindow7cc8c0handler_7cc8b4, 0x007cc8b4, 0x1);
GAME_DATA(u32, gWindow7cc8c0handler_7cc8d8, 0x007cc8d8, 0x4);
GAME_DATA(u8, gWindow7cc8c0handler_7cc8dc, 0x007cc8dc, 0x1);
GAME_DATA(u8, gWindow7cc8c0handler_7cc8e0, 0x007cc8e0, 0x1);
GAME_DATA(u8 *, gWindow7cc8c0handler_7cc8e8, 0x007cc8e8, 0x4);
