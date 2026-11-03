// A part that shows the text its owner's record names. Reimplementation of 0x492070 (a slot of the vtable near 0x6ff598); the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct TextRecordPartEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                   // Window_SendMessage
    u8 *(__fastcall *record)(const u8 *object, void *, u32 index);               // Records_DataIfTerminated
    u16 *(__cdecl *widen)(const u8 *text);                                       // Language_Widen
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);               // Window_NotifyC00
    void(__fastcall *notify100)(u8 *window, void *, u32 a);                      // Window_NotifyImpl100
    u32(__fastcall *mark)(u8 *part);                                             // Fn_669f80
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                    // Window_SetShown
};
void TextRecordPart_SetEnv(const TextRecordPartEnv *env);

extern "C" {

// 0x492070 (thiscall): nothing with a word +0xc or +4 of 0. The answer of the owner +0x14 to the message 0xc (0, 0); nothing with 0 or -1. Its text: +4 when set; else with +0 of -1 the text 0x78a374; else Records_DataIfTerminated of [0x78ba98] (+0) widened. The window +0x18 told Window_NotifyC00 (1, 0 with a newline in the text, else 1) and Window_NotifyImpl100 (the text), Fn_669f80 on the part; an empty text
// the part's slot 1, another the window shown.
void __fastcall TextRecordPart_Refresh(u8 *part);
}
