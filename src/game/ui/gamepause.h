// The game hidden behind a menu and brought back after the frame countdown (core/framecountdown). The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/buffers.h"

extern "C" {

// What these call (the bench points them at recorders): the ports; 0x4092a0, 0x409300, 0x5996d0, 0x5d47a0, 0x564430 (agent B's) and the CRT's atexit by address.
struct GamePauseEnv {
    u32(__cdecl *play)(u32 sound);                                       // SoundUi_Play 0x485aa0
    u8 *(__fastcall *readMode)(u8 *object);                              // Setting_ReadMode 0x550ca0
    u32(__cdecl *atExit)(u32 function);                                  // crt_atexit 0x67a7d0
    void(__fastcall *leave)(u8 *object, void *, u32 a, u32 b);           // 0x4092a0
    u32(__cdecl *choose)();                                              // Global7cc698_Choose 0x4bcec0
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);            // Window_SetShown 0x65d9dd
    void(__cdecl *cursorSlot3)();                                        // Gui_CursorSlot3 0x6591e3
    s32(__cdecl *finish)();                                              // ParticlePool2_Finish 0x5eda70
    void(__cdecl *fn5996d0)(u32 value);                                  // 0x5996d0
    void(__fastcall *enter)(u8 *object);                                 // 0x409300
    void(__cdecl *cursorSlot2)();                                        // Gui_CursorSlot2 0x659201
    void(__cdecl *cameraLeave)();                                        // CameraMode_Leave 0x48aff0
    u32(__cdecl *overlayState)();                                        // Global7cb2e8_Get 0x4a2a90
    u32(__cdecl *overlayToggle)();                                       // Overlay7cb2b4_Toggle 0x4a29d0
    void(__cdecl *selectFirst)();                                        // Windows7cc8bc_SelectFirst 0x4bcf50
    void(__cdecl *openDialog)();                                         // Dialog7cc184_Open 0x4a8c00
    void *(__cdecl *alloc)(u32 size);                                    // GameHeap_Alloc
    void(__cdecl *release)(void *block);                                 // GameHeap_Free
    void(__fastcall *blocksFree)(u8 *object);                            // Blocks7_Free 0x5d46d0
    u8 *(__fastcall *blocksCtor)(u8 *object);                            // Blocks7_Ctor 0x5d45b0
    void(__fastcall *fn5d47a0)(u8 *object);                              // 0x5d47a0
    void(__fastcall *fn564430)(u8 *object, void *, SurfaceDesc *desc);   // 0x564430
    u8(__fastcall *shrink)(u8 *object, void *, const SurfaceDesc *desc); // Blocks7_ShrinkSurface 0x5d48a0
    u32(__fastcall *storeCom)(u8 *object);                               // Obj_StoreComSlot80 0x5644c0
    void(__fastcall *panelReset)(u8 *panel);                             // Panel_Reset 0x5d48d0
};
// The calls go to env (nullptr: the defaults).
void GamePause_SetEnv(const GamePauseEnv *env);

// 0x4a9180 (cdecl; name invented): 0x7cc1a0 0. The setting 0xfb3b7c (read once, the guard bit 0xfb34bf) not 1: 0x786ba4 1, 0x409300 of 0x786b68. The picture remade (0x4a9280), the root
// 0xfaa5d8 shown, the cursor's slot 2, CameraMode_Leave. Setting 1: Windows7cc8bc_SelectFirst; else with Global7cb2e8_Get the overlay toggled. The temp path into 0x7cb958 once (0x7cc194),
// Dialog7cc184_Open, the flag 0x7cc190 0.
void __cdecl GamePause_Resume();
// 0x4a9280 (cdecl; name invented): the Blocks7 in 0x7cc18c freed and a new one (0x70) made; 0x5d47a0; the desc of [0x879a24] (0x564430) shrinks it; Obj_StoreComSlot80, Panel_Reset.
void __cdecl GamePause_RemakePicture();
// 0x4a9380 (cdecl; name invented): the sound 0x95. The setting not 1: 0x4092a0 of 0x786b68 (0, 0), 0x786ba4 0; 1: Global7cc698_Choose. The root hidden, the cursor's slot 3,
// ParticlePool2_Finish, 0x5996d0(1), the flag 0x7cc190 1 and the countdown 0x7cc150 3.
void __cdecl GamePause_Hide();
}
