// The handler of the window [0x7cb2b0] (made by src/game/ui/panel7b9940). The name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the handler calls (the benches point these at recorders): the ports.
struct Window7cb2b0HandlerEnv {
    u32(__cdecl *modifiers)();                                                     // KeyMap_Modifiers 0x46b420
    u32(__fastcall *fireUp)(u8 *table, void *, u32 a, u32 b);                      // Actions_FireUp 0x40c990
    u32(__fastcall *fireDown)(u8 *table, void *, u32 a, u32 b);                    // Actions_FireDown 0x40c900
    s32(__fastcall *findKey)(const u8 *map, void *, u32 key, u32 modifiers);       // KeyMap_FindKey 0x40c870
    void(__fastcall *setCursor)(u8 *window, void *, u32 id);                       // Window_SetCursor 0x65f11d
    void(__cdecl *walk)();                                                         // Camera_ToggleWalkMode 0x42ae70
    void(__cdecl *stopFollowing)();                                                // Camera_StopFollowing 0x42a190
    void(__cdecl *pauseHide)();                                                    // GamePause_Hide 0x4a9380
    void(__cdecl *camera)(u32 message, u32 a, u32 b);                              // Camera_Input 0x42a760
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault 0x65f66e
};
// The handler's calls go to env (nullptr: the defaults).
void Window7cb2b0Handler_SetEnv(const Window7cb2b0HandlerEnv *env);

// 0x488a00 (cdecl (window, message, a, b); name invented): 0x14: [0x7cb2b0] 0. 0x100, and 0x10006 with a 1 and the byte [0x78d911]: the camera let go. 0x10004 / 0x10005: the actions table
// [0x787c14] fired up / down for a - 0x10 with the modifiers held. 0x1000a: down for a with b's third byte. 0x1000b: up the same; then the camera let go for the key 0x1b or when the key map
// [0x78721c] finds it as 0x10 (AX), else GamePause_Hide when it (asked again) finds it as 0xf. Letting go, with any of 0x16 in [0x790ab0]: the cursor 1, by the same byte's bit 2 the walk
// mode toggled or else by bit 4 the following stopped, the cursor 0x15. Every message then goes to Camera_Input and Window_CallDefault, its answer returned.
s32 __cdecl Window7cb2b0_Handler(u8 *window, u32 message, u32 a, u32 b);
}
