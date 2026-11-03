// The online dialog's list of dial-up connections. Reimplementation of 0x4ba850; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the imports through their slots, 0x67c430 raw: unported; the bench's recorders: their own benches have them).
struct DialupListEnv {
    u32(__fastcall *count)(void *dialup);                                 // WEARASR's Dialup::GetNoDialUpConnections
    u32(__fastcall *properties)(void *dialup, void *, u32 index, void *out);  // WEARASR's Dialup::GetDialupProperties
    void(__fastcall *clear)(u8 *window);                                  // Window7059f0_Clear
    void(__fastcall *setShown)(u8 *window, void *, u32 shown);            // Window_SetShown
    s32(__fastcall *add)(u8 *window, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);  // Window7059f0_AddItem
    void(__fastcall *refresh)(u8 *window);                                // Window7059f0_RefreshRows
    u16 *(__cdecl *wideText)(u32 id);                                     // Language_WideText
    u16 *(__cdecl *token)(u16 *text, const u16 *separators);              // 0x67c430
};
void DialupList_SetEnv(const DialupListEnv *env);

extern "C" {

// 0x4ba850 (cdecl (shown, a byte)): the list child 0xc of [0x7cc530] cleared. Not shown: its Window_SetShown (0), nothing more. Else shown and, with the object 0x7c24c0 set up ([0x7c24d4] not 0), a row for each of its connections whose properties' dword +0xc has bit 0 (the text at +0x10, its dword +0 the text's second, data
// the index, parent -1); without any connections the words of the language text 0xe6, not set up those of 0xe7 (0x67c430 on the buffer 0x7ba058 at the line ends 0x7558f0; data -1, second 0). The rows refreshed.
void __cdecl DialupList_Fill(u8 shown);
}
