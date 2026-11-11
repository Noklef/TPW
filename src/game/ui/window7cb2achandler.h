// The handler of the window [0x7cb2ac] (the park view, made by src/game/ui/panel7b9940). Reimplementation of 0x4881a0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: driver.cpp's kWindow7cb2acHandlerEnv). The mouse modes' holder and the drag's mode run as they are.
struct Window7cb2acHandlerEnv {
    u32(__fastcall *capture)(u8 *window);                                          // Window_CaptureMouse
    u32(__fastcall *release)(u8 *window);                                          // Window_ReleaseMouse
    u32(__cdecl *clock)();                                                         // GuiClock_Now
    void(__cdecl *camera)(u32 message, u32 a, u32 b);                              // Camera_Input
    void(__cdecl *gameMode)(u32 mode, u32 record);                                 // GameMode_Set
    void(__cdecl *hide)();                                                         // Dialog7ca1c8_Hide
    void(__stdcall *moved)(s32 x, s32 y);                                          // Mouse_Moved
    void(__cdecl *unhighlight)();                                                  // Highlight_Clear
    void(__cdecl *help)(u32 forced);                                               // MouseHelp_Update
    u32(__cdecl *modifiers)();                                                     // KeyMap_Modifiers
    u32(__fastcall *fireDown)(u8 *table, void *, u32 a, u32 b);                    // Actions_FireDown
    u32(__fastcall *fireUp)(u8 *table, void *, u32 a, u32 b);                      // Actions_FireUp
    void(__cdecl *button)(u32 button, u32 down, s32 x, s32 y, u32 zero);           // Mouse_HandleButton
    u32(__cdecl *shown)();                                                         // MouseClick_OnShown
    u8 *(__fastcall *cheatsCtor)(u8 *cheats);                                      // CheatSystem_Construct
    s32(__cdecl *atExit)(void *fn);                                                // CRT_atexit
    u8(__fastcall *cheatsOn)(const u8 *cheats);                                    // GetBytea_405560
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
};
void Window7cb2acHandler_SetEnv(const Window7cb2acHandlerEnv *env);

extern "C" {

// 0x4881a0 (cdecl (window, message, a, b); set by Panel7b9940_Open). [0x74fc2c] (the message to the camera) 1. With the byte [0x78d911] clear: 0x10005 / 0x10004 with a == 1 the mouse
// captured / released. With it set, a drag ([0x7c2500]): 0x10005 with a == 1 captures, makes the holder, calls the mode [0x7b05d8]'s slot 9, starts the drag (its clock [0x7c2504], b's
// words at 0x7b5344 / 0x7b9774) and clears [0x74fc2c]; 0x10004 with a == 1 releases and, in a drag, sets a new 8-byte online mode (holder made), GameMode_Set(0, 1) and Dialog7ca1c8_Hide;
// the drag ends. 0x1e after 0xc8 ticks and 0x10003 with a 9 or more from b's words end a drag with Camera_Input(0x10005, 1, 0). Then Camera_Input(message, a, b) when [0x74fc2c], and:
// 0x14: [0x7cb2ac] 0. 0x15: [0x7c2500] and [0x7c24fc] 0. 0x1e: the holder made, Mouse_Moved by the cursor [0xfaa540] / [0xfaa544] scaled ([0xfaa5c4] / 2048, [0xfaa5c0] / 1536),
// Highlight_Clear unless [0x7a1c7c], MouseHelp_Update(0) with no [0x7c24c8] and [0x7c24fc] set. 0x10001: [0x7c24f8] 0, MouseHelp_Update(0) with no [0x7c24c8], [0x7c24fc] 1, [0x7c24f4] 0.
// 0x10002: [0x7c24fc] 0. 0x10004: Actions_FireUp([0x787c14], a - 0x10, modifiers), the holder made, Mouse_Moved by b scaled, Mouse_HandleButton(0xf0 + (a != 0), 0, x, y, 0).
// 0x10005: Actions_FireDown the same; then with no [0x7c24c8] and a != 2 (a == 0: MouseHelp_Update(0) and MouseClick_OnShown's answer set) the holder made and
// Mouse_HandleButton(0xf0 + (a != 0), 1, x, y, 0). 0x1000a / 0x1000b: Actions_FireDown / Up (a, b's byte 2) on [0x787c14], [0x787a60] and [0x78721c] until one answers, then on the
// cheat map [0x78733c] when the cheats (0xfb3520, made once: guard 0xfb34c8 bit 0, atexit 0x5d1750) are on. Answers Window_CallDefault's.
s32 __cdecl Window7cb2ac_Handler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gWindow7cb2achandler_74fc2c, 0x0074fc2c, 0x4);   // the message goes to the camera (1 at the start, 0 when a drag starts)
