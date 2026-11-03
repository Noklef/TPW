// The GUI's handler of every window's messages (given to Fn_6590ea at start-up): the click's sound, the IME on a text window, the particles on a button. Reimplementation of 0x485780; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct GuiMessageFxEnv {
    u32(__cdecl *playAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);   // Sound_PlayAt
    void(__fastcall *attach)(u8 *host, void *, u8 *window);                          // ImeHost_Attach
    void(__fastcall *detach)(u8 *host);                                              // ImeHost_Detach
    void(__fastcall *onWindow)(u8 *host, void *, u8 *window);                        // ImeHost_OnWindow
    u8 *(__fastcall *findChild)(u8 *window, void *, u32 id);                         // Window_FindChild
    u16(__fastcall *getRect)(u8 *window, void *, u8 *rect, u32 inner);               // Window_GetRect
    s32(__cdecl *finish)();                                                          // ParticlePool2_Finish
    s32(__cdecl *empty)();                                                           // ParticlePool2_Empty
    u32(__cdecl *position)(u32 x, u32 y);                                            // Fn_5eda40: the bits of two floats
    void(__cdecl *setup)(u32 x, u32 y, float width, float height);                   // ParticlePool2_Setup: x and y the bits of two floats
};
void GuiMessageFx_SetEnv(const GuiMessageFxEnv *env);

extern "C" {

// 0x485780 (cdecl (window, message, a, b)): 0. 0x100: Sound_PlayAt (0, [0x803a2c], 0xbd with the look bit 0x10 of +0x48, else 0x1f, 0, 0, 0). A window of the kind 5 (slot 3): 0x1f attaches the IME host 0x7b4e38, 0x14 and 0x20 detach it;
// 0x801 tells it of the child a (any kind). With the byte [0x78d90e], a window of the kind 2: 0x10001 sets the pool 2 up from the inner rectangle (centre less 0x400, 0x300 over 1024, 768; size over 2048, 1536); 0x13, with the state word 1,
// moves it to the centre; 0x11 with the flag +0x139 at 1, 0x14 with that flag or the state word 1: ParticlePool2_Finish; 0x10002: ParticlePool2_Empty.
u32 __cdecl GuiMessageFx_Handler(u8 *window, u32 message, u32 a, u32 b);
}
