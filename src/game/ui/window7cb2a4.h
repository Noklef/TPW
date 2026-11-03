// The window [0x7cb2a4] (made by Dialog7cb2b8_Build): its handler. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4a0fb0 (cdecl; window, message, a, b; name invented, unconfirmed). Message 0x15: the remembered counts [0x752ec4] and [0x752ec8] = -1. Message 0x1e: unless the setting [0xfb3b7c] (guard 0xfb34c1 bit 0, Setting_ReadMode, atexit 0x5d1750) is 2,
// the save's GlobalSave_CountSetLevel, when it is not [0x752ec8], remembered: 0 hides the children 0x36 and 0x34; else both shown and 0x36's slot 9 given the count (swprintf with the format 0x752ef8). Then, whatever the setting, GlobalSave_CountSetLeft
// the same way with [0x752ec4] and the children 0x37 and 0x35. Always ends in Window_CallDefault, whose answer it returns.
s32 __cdecl Window7cb2a4_Handler(u8 *window, u32 message, u32 a, u32 b);
}

// The handler's calls (the bench's recorders; their own benches have them). Window_FindChild and Window_CallDefault run.
struct Window7cb2a4Env {
    u8 *(__fastcall *readMode)(u8 *object);                                  // Setting_ReadMode
    s32(__cdecl *atExit)(void(__cdecl *stub)());                             // crt atexit 0x67a7d0
    u8 *(__cdecl *save)();                                                   // GlobalSave_Instance
    s32(__fastcall *level)(const u8 *info);                                  // GlobalSave_CountSetLevel
    s32(__fastcall *left)(const u8 *info);                                   // GlobalSave_CountSetLeft
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                // Window_SetShown
    s32(__cdecl *format)(u16 *to, const u16 *format, s32 value);             // swprintf 0x67af00
};
void Window7cb2a4_SetEnv(const Window7cb2a4Env *env);
