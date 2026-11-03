// The IME host taken to a text window and off it. Reimplementation of 0x480ec0 and 0x480f50; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; 0x47a3a0 and 0x47a570 unported, agent B's, by address; the bench's recorders).
struct ImeHostAttachEnv {
    void(__fastcall *open)(u8 *ime, void *, s32 *result, u32 handle);        // 0x47a3a0
    void(__fastcall *close)(u8 *ime);                                        // 0x47a570
    u16(__fastcall *textLength)(const u8 *window);                           // Window705a50_TextLength
    void(__fastcall *setLimit)(u8 *ime, void *, u32 limit);                  // Ime_SetLimit
    u32(__fastcall *windowOff)(u8 *window, void *, u32 v0);                  // Fn_668564
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);               // Window_SendMessage
};
void ImeHost_SetAttachEnv(const ImeHostAttachEnv *env);

extern "C" {

// 0x480ec0 (thiscall (window)): the IME +0x1c opened (0x47a3a0 (&result, [0x7a130c])). A negative result: the byte +0xc 0. Else the byte 1, +4 the window and twice its limit (Ime_SetLimit) its word +0x13c less Window705a50_TextLength as signed words, the words +0x1a8, +0x1aa 0, 1 between them.
void __fastcall ImeHost_Attach(u8 *host, void *, u8 *window);
// 0x480f50 (thiscall): the IME closed (0x47a570); the window +4, when set, told Fn_668564 (0) and +4 0; the window +0, when set, sent 4 (0, 0); the byte +0xc 0.
void __fastcall ImeHost_Detach(u8 *host);
}
