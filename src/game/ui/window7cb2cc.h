// The window [0x7cb2cc]'s handler. Reimplementation of 0x4a2680; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; 0x409350 unported, agent B's, by address; the bench's recorders).
struct Window7cb2ccEnv {
    s32(__fastcall *findKey)(const u8 *map, void *, s16 key, s16 modifiers);       // KeyMap_FindKey
    void(__fastcall *key)(u8 *keys, void *, u32 a);                                // 0x409350
    u16(__fastcall *getRect)(u8 *window, void *, u8 *rect, u32 inner);             // Window_GetRect
    void(__fastcall *setWords)(u8 *window, void *, u32 a, u32 b);                  // Window_SetWordsE4
    u16(__fastcall *runScript)(u8 *window, void *, u8 *script, s32 interval);      // Window_RunScript
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);                     // Dialog_BaseHandler
};
void Window7cb2cc_SetEnv(const Window7cb2ccEnv *env);

extern "C" {

// 0x4a2680 (cdecl (window, message, a, b)): 0x14: [0x7cb2cc] = 0. 0x15: the window's rectangle (Window_GetRect (1)), its words +8 and +0xa halved (the words' arithmetic shifts, the registers' upper halves pushed as they are) into Window_SetWordsE4, the script 0x752ed0 run (0x23). 0x1000b: with the word [[0x78718c]+6] 0, KeyMap_FindKey of [0x78718c] (a, modifiers 1 unless b has any of 0xff0000) and, its low word not 0, 0x409350 on 0x786b68 (0).
// The answer always Dialog_BaseHandler's.
s32 __cdecl Window7cb2cc_Handler(u8 *window, u32 message, u32 a, u32 b);
}
