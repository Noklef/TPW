// The game window's handler: the keys and mouse buttons. The name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the handler calls (the bench points these at recorders): the ports; 0x4a9380 by address.
struct GameWindowInputEnv {
    u32(__fastcall *capture)(u8 *window);                             // Window_CaptureMouse 0x65e549
    u32(__fastcall *release)(u8 *window);                             // Window_ReleaseMouse 0x65e56e
    void(__cdecl *camera)(u32 message, u32 a, u32 b);                 // Camera_Input 0x42a760
    u32(__cdecl *modifiers)();                                        // KeyMap_Modifiers 0x46b420
    u32(__fastcall *fireUp)(u8 *table, void *, u32 a, u32 b);         // Actions_FireUp 0x40c990
    u32(__fastcall *fireDown)(u8 *table, void *, u32 a, u32 b);       // Actions_FireDown 0x40c900
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault 0x65f66e
    void(__cdecl *openPanel)(u32 flag);                               // Panel7c2534_Open 0x48c830
    s32(__fastcall *findKey)(const u8 *map, void *, u32 key, u32 modifiers);       // KeyMap_FindKey 0x40c870
    u32(__cdecl *toggleOverlay)();                                    // Overlay7cb2b4_Toggle 0x4a29d0
    void(__cdecl *key15)();                                           // 0x4a9380
};
// The handler's calls go to env (nullptr: the defaults).
void GameWindowInput_SetEnv(const GameWindowInputEnv *env);

// 0x4a2840 (cdecl (window, message, a, b); name invented): 0x10005 with a 1 captures the mouse, 0x10004 with a 1 releases it; every message goes to Camera_Input. 0x10004 and 0x10005 fire
// the actions [0x787c14] up and down for a - 0x10 with the modifiers, 0x1000a fires down for a with b's third byte; each then the default handler, as all the rest but 0x1000b. It fires
// up the same and answers 0: in the game's mode 4 Escape without modifiers opens the panel; else action 4 of the map [0x787a60] or Escape without modifiers toggles the overlay, and
// action 0xf of the map [0x78721c] calls 0x4a9380.
s32 __cdecl GameWindow_Input(u8 *window, u32 message, u32 a, u32 b);
}
