// What the window at 0x7cc47c shows of an item: its number and its text (reimplementation of 0x4b8850). The name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/stdwstring.h"

extern "C" {

// The surroundings the bench replaces.
struct Dialog7cc47cInfoEnv {
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                 // Window_SetShown
    s32(__cdecl *format)(u16 *to, const u16 *format, s32 value);              // swprintf 0x67af00
    void(__fastcall *censor)(u8 *save, void *, StdWString *text, u32 flag);   // 0x5d3230 (B)
};
void Dialog7cc47cInfo_SetEnv(const Dialog7cc47cInfoEnv *env);   // nullptr: the game's

// 0x4b8850 (cdecl): no window [0x7cc47c]: nothing. An item with a +0x70: the children 0x49da and 0x49d9 shown (each looked up from [0x7cc47c]),
// 0x49da told (slot 9) the wide buffer 0x7ba058 formatted with 0x74f9d8 from the item's +0x24 plus 1; the text at (+0x70)+0x14 copied into a
// local wide string (_Grow(n, true), inlined the rest), censored (0x5d3230 on the global save info, (&text, 1)) when the info's byte +0x25
// (GlobalSave_Byte25) is set, and 0x49d9 told its characters (null: 0x6fd6c4). Otherwise (no item, or no +0x70) both children hidden.
void __cdecl Dialog7cc47cInfo_Show(const u8 *item);

}  // extern "C"
