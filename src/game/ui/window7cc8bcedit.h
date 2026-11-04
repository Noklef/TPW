// The handler of the online view's edit box 0xc312c (the chat line). Reimplementation of 0x4be3a0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/stdwstring.h"

// The calls (the ports by default, agent A's unported 0x4ea490 by address; tests/fncmp/window7cc8bcedit.py gives recorders).
struct Window7cc8bcEditEnv {
    s32(__fastcall *findKey)(const u8 *map, void *, u32 key, u32 modifiers);    // KeyMap_FindKey
    u32(__fastcall *killFocus)(u8 *window, void *, u32 a);                      // Window_KillFocus
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                       // Window_SetFocus
    u8 *(__fastcall *holderCtor)(u8 *holder);                                   // MouseModeHolder_Ctor
    u8 *(__fastcall *onlineCtor)(u8 *self);                                     // MMOnline_Ctor
    u8 *(__fastcall *baseCtor)(u8 *self);                                       // MouseModeOnlineBase_Ctor
    void(__fastcall *setMode)(u8 *holder, void *, u8 *mode);                    // MouseMode_Set
    void(__fastcall *setCursor)(u8 *window, void *, u32 id);                    // Window_SetCursor
    void(__cdecl *openPanel)(u32 flag);                                         // Panel7c2534_Open
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                  // Window_SendMessage
    u8 *(__cdecl *chat)();                                                      // OnlineChat_Instance
    void(__fastcall *say)(u8 *chat, void *, StdWString text);                   // 0x4ea490 (the callee owns text)
    void(__cdecl *resetKeys)();                                                 // KeyMaps_ResetAllAndSetFlag
    void(__cdecl *enableKeys)();                                                // KeyMaps_EnableAllAndClearFlag
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
};
void Window7cc8bcEdit_SetEnv(const Window7cc8bcEditEnv *env);   // nullptr: the game's

extern "C" {

// 0x4be3a0 (cdecl (window, message, a, b)). Message 0x1f: KeyMaps_ResetAllAndSetFlag; 0x20: KeyMaps_EnableAllAndClearFlag. 0x1000b, a key a with the modifiers byte 2 of b: 9: the focus killed and given to [0x7cc8bc], answer 1. 0xd unmodified: the text [0x7cc6a8] said (0x4ea490 on OnlineChat_Instance), the window's slot 9 (0x78a374), answer 1.
// 0x73 unmodified with [0x7cc6a0] 0: [0x7cc8bc] and [0x7cc8c0] sent 6 (![0x7cc8b4], [0x7cc8b4]); with [0x7cc8b4] set the focus to child 0xc3123 / 0xc312b / 0xc312c of [0x7cc8bc], [0x7b05e8] made if none, in mouse mode 9 (slot 9 of [0x7b05d8]) cursor 0x13 on [0x7cc8bc]; else the focus to [0x7cc8c0].
// Any other key KeyMap_FindKey on [0x78721c] does not know: the focus killed and given to [0x7cc8bc], [0x7b05e8] made if none; in mouse mode 9 a new MMOnline set on it (it being none: made with a new base, [0x7b05e0] and [0x7b05d4] -1) and cursor 0 on [0x7cc8c0], else Panel7c2534_Open(0).
// All the rest: Window_CallDefault, its answer.
s32 __cdecl Window7cc8bc_EditHandler(u8 *window, u32 message, u32 a, u32 b);
}
