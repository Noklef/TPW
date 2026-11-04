// The online game's list given its waiting row's text. Reimplementation of 0x4c0660; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cc944RowEnv {
    u8 *(__fastcall *findChild)(u8 *window, void *, u32 id);                                         // Window_FindChild
    void(__fastcall *itemData)(u8 *list, void *, u32 n, u32 *out);                                   // Window7059f0_NthItemData
    s32(__cdecl *swprintf)(u16 *to, const u16 *format, ...);                                         // CRT_swprintf
    void(__fastcall *setTexts)(u8 *list, void *, u32 n, const u32 *texts, u32 mask, u32 refresh, s32 unused);   // Window7059f0_SetItemTexts
};
void Dialog7cc944Row_SetEnv(const Dialog7cc944RowEnv *env);

extern "C" {

// 0x4c0660 (cdecl (value)): with the dialog [0x7cc944] up, when its list 0xe05b's first row has the data -1: the value by the format 0x759160 into 0x7ba058, that the row's second text (mask 2).
void __cdecl Dialog7cc944_SetWaitingRow(u32 value);
}
