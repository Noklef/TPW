// A list item's texts set (names invented, unconfirmed): reimplementation of 0x6644ea and the library's string assign 0x665f10, 0x665f80.
#pragma once

#include "../../../re/types.h"

struct Window7059f0TextsEnv {
    u8 *(__fastcall *assign)(u8 *string, void *, const u16 *chars, u32 count);   // 0x4051d0 (B): std::wstring::assign(chars, n)
    s32(__fastcall *nth)(u8 *window, void *, s32 n);                             // Window7059f0_NthItem
    void(__fastcall *refresh)(u8 *window);                                       // Window7059f0_RefreshRows
};
void Window7059f0Texts_SetEnv(const Window7059f0TextsEnv *env);

extern "C" {

// 0x665f80 (thiscall (text), pops 4): assign(text, WTraits_Length(text)); its answer.
u8 *__fastcall WString_AssignText(u8 *string, void *, const u16 *text);
// 0x665f10 (thiscall (text), pops 4): WString_AssignText.
u8 *__fastcall WString_AssignTextOf(u8 *string, void *, const u16 *text);
// 0x6644ea (thiscall (n, texts, mask, refresh, unused), pops 20): nothing without an item array +0x144, without texts or for n from the count +0x150 on (signed). The n-th item (Window7059f0_NthItem; none if negative): for each column up to the
// word +0x164 with its bit in mask (shifted by the column mod 32), a cell of the item (the array's dwords from item + 4): with a column bottom (Window7059f0_ColumnBottom) other than 0 the text itself, else a set text assigned to a set cell.
// Any such column and refresh: Window7059f0_RefreshRows.
void __fastcall Window7059f0_SetItemTexts(u8 *window, void *, s32 n, const u32 *texts, u32 mask, u32 refresh, u32 unused);
}
