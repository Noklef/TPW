// The dialog [0x7cc184]'s handler and that of its picture (ui/dialog7cc184 opens it). Reimplementation of 0x4a7d70, 0x4a7c50 (names invented, unconfirmed).
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default, the CRT's unported ones by address; the bench's recorders: their own benches have them).
struct Dialog7cc184HandlerEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                      // Window_SendMessage
    void(__fastcall *freeParts)(u8 *object);                                        // Blocks7_Free
    s32(__cdecl *atExit)(void(__cdecl *f)());                                       // 0x67a7d0
    s32(__cdecl *pair)();                                                           // Windows7cc8bc_Message6Pair
    void(__cdecl *cancel)(u32 id);                                                  // Game_CancelAdvisor
    u32(__cdecl *focus)();                                                          // Focus_Restore
    void(__cdecl *enableKeys)();                                                    // KeyMaps_EnableAllAndClearFlag
    void(__cdecl *resetKeys)();                                                     // KeyMaps_ResetAllAndSetFlag
    void(__cdecl *openAddress)();                                                   // Dialog7cc188_Open
    u8 *(__fastcall *find)(u8 *window, void *, u32 id);                             // Window_FindChild
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry);              // Charset_FormatWide
    void(__cdecl *show)(u8 *window);                                                // TextPanel7cc18c_Show
    u16 *(__cdecl *token)(u16 *text, const u16 *separators);                        // 0x67c430
    u16 *(__cdecl *wideText)(u32 index);                                            // Language_WideText
    void(__cdecl *box)(const void *record, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g);   // 0x47f020
    void(__cdecl *sendCard)();                                                      // Postcard_Send
    s32(__cdecl *keys)(u8 *window, u32 message, u32 a, u32 b);                      // Dialog_KeyHandler
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);    // Window_CallDefault
    u32(__fastcall *capture)(u8 *window);                                           // Window_CaptureMouse
    u32(__fastcall *release)(u8 *window);                                           // Window_ReleaseMouse
};
void Dialog7cc184Handler_SetEnv(const Dialog7cc184HandlerEnv *env);

extern "C" {

// 0x4a7c50 (cdecl (window, message, a, b)): the picture 0x5d9f's handler. 0x10005 (b: y high, x low): dragging ([0x7cc1a4] = 1) from there ([0x7cc1a8], [0x7cc1ac]; the x unsigned), the mouse captured. 0x10004: no more, released. 0x10003 (a) while dragging: the offsets [0x7cc198] += (last y - y) * the height word [0xfaa5c4] / 2048, [0x7cc19c] += (last x - x) * the width word [0xfaa5c0] / 1536 (signed, each truncated),
// both at least 0, the position kept. Always then Window_CallDefault.
s32 __cdecl Dialog7cc184_PictureHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x4a7d70 (cdecl (window, message, a, b)): 5: the dialog sent (4, 0, 0), 1. 0x14: [0x7cc184] = 0, the panel [0x7cc18c] freed (Blocks7_Free, Mem_Free) and 0; in the mode setting 1 (the guard 0xfb34bf bit 0, Setting_ReadMode, atexit 0x5d1750) Windows7cc8bc_Message6Pair, else Game_CancelAdvisor (0x133); Focus_Restore, the keys enabled. 0x15: the keys reset.
// 0x100 with a: 0x5da0: Dialog7cc188_Open. 0x5da1: the children 0x5da3, 0x5da4, 0x5da5 given the empty text 0x78a374 (slot 9). 0x5da2: TextPanel7cc18c_Show (child 0x5d9f). -2: the dialog sent (4, 0, 0), 0. -1: the two texts not typed ([0x7cbf40], [0x7cbf3c]) from the language (0x7cbf48, 0x100, 0xd9; 0x7cb408, 0x60, 0xd8); the panel shown; the addresses [0x7cb4d0], copied,
// split on 0x754448, each needing an '@' past its start and a '.' more than one place past that: the text not empty and all good: Postcard_Send and the dialog sent (4, 0, 0); else the box 0x47f020 with the text 0xd6. [0x7cc1a0] = 1, 0. 0x801 with a: 0x5da3: [0x7cbf40] = 1; 0x5da4: [0x7cbf3c] = 1. What is not answered above goes on, by the mode setting at 1, to Window_CallDefault, else Dialog_KeyHandler.
s32 __cdecl Dialog7cc184_Handler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0xc], gDialog7cc184handler_754448, 0x00754448, 0xc);
GAME_DATA(u8 *, gDialog7cc184handler_7cc184, 0x007cc184, 0x4);
GAME_DATA(u8 *, gDialog7cc184handler_7cc18c, 0x007cc18c, 0x4);
GAME_DATA(s32, gDialog7cc184_PictureHandler_7cc198, 0x007cc198, 0x4);
GAME_DATA(s32, gDialog7cc184_PictureHandler_7cc19c, 0x007cc19c, 0x4);
GAME_DATA(u32, gDialog7cc184handler_7cc1a0, 0x007cc1a0, 0x4);
GAME_DATA(u32, gDialog7cc184_PictureHandler_7cc1a4, 0x007cc1a4, 0x4);
GAME_DATA(s32, gDialog7cc184_PictureHandler_7cc1a8, 0x007cc1a8, 0x4);
GAME_DATA(s32, gDialog7cc184_PictureHandler_7cc1ac, 0x007cc1ac, 0x4);
