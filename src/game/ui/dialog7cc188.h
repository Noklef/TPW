// The list of the dialog [0x7cc188] (its child 0x26). Reimplementation of 0x4a7a80; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cc188ListEnv {
    u8 *(__cdecl *source)();                                                             // 0x48bcd0 (its constructor agent B's)
    s32(__cdecl *swprintf)(u16 *to, const u16 *format, const char *text);                // 0x67af00
    s32(__cdecl *atExit)(void(__cdecl *f)());                                            // 0x67a7d0
    void(__fastcall *clear)(u8 *list);                                                   // Window7059f0_Clear
    s32(__fastcall *add)(u8 *list, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);   // Window7059f0_AddItem
    void(__fastcall *refresh)(u8 *list);                                                 // Window7059f0_RefreshRows
    u16 *(__cdecl *wideText)(u32 id);                                                    // Language_WideText
};
void Dialog7cc188_SetListEnv(const Dialog7cc188ListEnv *env);

extern "C" {

// 0x4a7a80 (cdecl (clear)): the list child 0x26 of [0x7cc188], emptied when clear is not 0, given the strings of the list +0x100 of 0x48bcd0's object up to the first empty one (a null text is the empty string 0x6fe5b0), each formatted "%.63S" on the stack, its data its index. The mode setting (the
// guard 0xfb34bf bit 0, Setting_ReadMode, atexit 0x5d1750) 1, and the session's Obj_Get3a4's wide string +0x9c set and not empty: one more, the text 0x121, data -1. No refresh by the adds: the rows refreshed last.
void __cdecl Dialog7cc188_FillList(u32 clear);
}
