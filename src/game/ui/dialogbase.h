// The game's dialogs: one made from a window script (0x47ed80, called by 0x4a27f0) and the message handler they fall back on (0x47ee90, called by the handlers at 0x49e8ec, 0x49e97f, 0x49ec6d, 0x4a2768,
// 0x4a6d3f, 0x4a6e2d, 0x4b9f16, 0x4ba72b, 0x4bb05f and 0x4bb715). The name is unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// Dialog_Box's and Dialog_BoxHandler's calls by address (the bench's recorders).
void Dialog_SetCalls(const AddrCall *table, u32 count);

// Dialog_KeyHandler's calls (the ports by default, 0x59ab50 raw: unported, agent B's; the bench's recorders: their own benches have them).
struct DialogKeyEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);               // Window_SendMessage
    u8 *(__fastcall *advisor)(const u8 *game);                               // GameTable7cfb90_Get1da71c_5194d0
    u32(__fastcall *helpKey)(u8 *advisor);                                   // 0x59ab50
    u32(__fastcall *fireDown)(u8 *table, void *, u32 a, u32 b);              // Actions_FireDown
    u32(__fastcall *fireUp)(u8 *table, void *, u32 a, u32 b);                // Actions_FireUp
    u32(__cdecl *setWindow)(u8 *window);                                     // Window7c24c8_Set
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
};
void Dialog_SetKeyEnv(const DialogKeyEnv *env);

extern "C" {

// 0x488ba0 (cdecl (window, message, a, b); name invented): the dialogs' handler for keys. 0x1000b (a key up, its code a, b's third byte the state): state 0 and code 0x1b: the window sent 4 (0, 0), answer 0; state 0 and code 0x70: 0x59ab50 on the game's advisor, 0; else Actions_FireUp (code, the state) of [0x78721c], 0.
// 0x1000a: Actions_FireDown the same way, 0. 0x14: Window7c24c8_Set (0). The rest, and 0x14 after that, Window_CallDefault, whose answer is returned.
s32 __cdecl Dialog_KeyHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x47ee90 (cdecl (window, message, a, b)): on message 0x14 the window's parent (+0x60, Fn_65d55a) is posted message 4 (0, 0)
// (Window_PostMessage) and the focus given back (Focus_Restore); then the window's default handler (Window_CallDefault) with the
// message, its result returned.
s32 __cdecl Dialog_BaseHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x47ed80 (cdecl (script, handler, shaded)): a frame made as a child of the GUI root [0xfaa5d8] (Window_CreateChild kind 1, flags 1,
// id 1, 0, 0 - 0x7ff, 0x5ff) and the script built in it (WindowScript_Build with the handler); the depth: shaded, the top one of the GUI
// (Gui_TopDepth(0, 0)) and the frame given the implementation named "lolight" (GuiWindows_FindByName, Window_SetImpl); else the top
// one over the built window's inner rectangle (Window_GetRect into a GuiRect on the stack). The frame at that depth + 1
// (Window_SetDepth), the built window given the handler when not 0 (Window_SetHandler) and the focus (Window_SetFocus(0)). Returns the
// built window, or 0 when the frame or the script made none.
u8 *__cdecl Dialog_Create(u8 *script, u32 handler, u32 shaded);

// 0x47f020 (cdecl (record, kind, callback, kind, callback, kind, callback, keys kept); name invented, unconfirmed): the message box: Dialog_Create(the script 0x74f920, Dialog_BoxHandler, 1) (0 returned
// when none), a text part made its implementation (none when the memory fails) and two calls with (1, 1); the record gives the font (+8 through Language_Font), the colour (+4, +5, +6, signed; alpha 0xff),
// the text (+0) and the callbacks' value (+0xc, its slot 3). The three buttons (children 0x9873c8-0x9873ca): kind 0 hidden, else given the implementation by kind (1 and 2 named, others none), shown and
// their callback in slots 0-2. Slot 4: -1 when the keys are kept, else the byte Global7c24d0_GetByte and the keys reset. Slot 5: 0; while the game was paused by a box ([0x786ba4]) and [0xf82884] is 0, the
// game owner paused (0, 0), the flag cleared, 0x662411 called and the slot 1. Returns the window.
u8 *__cdecl Dialog_Box(const u8 *record, u32 kind0, u32 callback0, u32 kind1, u32 callback1, u32 kind2, u32 callback2, u8 keysKept);

// 0x47eed0 (cdecl (window, message, a, b); name invented, unconfirmed): the box's handler. 0x100 from a button (a): its callback (slot 0-2; none counts as 1) given slot 3; a true answer posts the window
// message 4 (0, 0) and returns 0. 0x14: slot 4 unless -1 gives the keys back (true: KeyMaps_ResetAllAndSetFlag, else KeyMaps_EnableAllAndClearFlag); slot 5 true, [0x786ba4] and [0xf82884] 0: the flag set,
// the game owner resumed, 0x662420 called; then the parent posted message 4 and the focus given back. The rest, and 0x14 after that, Window_CallDefault, whose answer is returned.
s32 __cdecl Dialog_BoxHandler(u8 *window, u32 message, u32 a, u32 b);
}
