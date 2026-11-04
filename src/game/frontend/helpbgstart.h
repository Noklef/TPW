// The help background started. Reimplementation of 0x492470; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; the bench's recorders: their own benches have them).
struct HelpBgStartEnv {
    void(__fastcall *build)(u8 *self);                          // HelpBg_Build
    u32(__fastcall *stop)(u8 *self);                            // Frame_Stop
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);   // Window_SetShown
    void(__cdecl *setPointer)(u8 *object);                      // Gui_SetPointer48c
};
void HelpBgStart_SetEnv(const HelpBgStartEnv *env);

extern "C" {

// 0x492470 (cdecl): the help background 0x7ca0c0 built; with its word +0xc set, stopped and its window [0x7ca0d8] hidden; it made the GUI's pointer.
void __cdecl HelpBg_Start();
}
