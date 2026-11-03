// The dialog at 0x7ca308 (the window made from the template 0x751fa8 (id 0x247f) with the handler 0x49b650 by 0x49bf31; its child 0x2484
// shows the chosen staff member's figure, the script slot [0x7ca310]). Reimplementation of 0x49ba00. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// Dialog7ca308_FillList's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7ca308Env {
    void(__fastcall *clear)(u8 *list);                                                        // Window7059f0_Clear
    s32(__fastcall *addItem)(u8 *list, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);   // Window7059f0_AddItem
    void(__fastcall *refresh)(u8 *list);                                                      // Window7059f0_RefreshRows
    void(__fastcall *formatName)(const u8 *entry, void *, u16 *to, u32 size);                 // StaffPoolEntry_FormatName
    s32(__fastcall *wage)(const u8 *record);                                                  // Staff_WageForRecord
    s32(__fastcall *findItem)(u8 *list, void *, u32 data);                                    // Window7059f0_FindItemData
    u32(__fastcall *removeItem)(u8 *list, void *, s32 n);                                     // Window7059f0_RemoveItem
    void(__cdecl *fillList)(u32 kind, u32 clear);                                             // Dialog7ca308_FillList (Open's)
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);                   // Charset_FormatWide (Open's)
    u16(__fastcall *selectRadio)(u8 *group, void *, u32 id);                                  // RadioGroup_Select (Open's)
    void(__cdecl *attachLabel)(u8 *parent, u32 id, u32 text);                                 // HoverLabel_Attach (Open's)
    void(__cdecl *killScript)(s32 slot);                                                      // SpriteScript_Kill (ListHandler's)
    u16 *(__cdecl *wideText)(u32 index);                                                      // Language_WideText (ListHandler's)
};
void Dialog7ca308_SetEnv(const Dialog7ca308Env *env);

extern "C" {

// 0x49b5b0 (cdecl (kind, clear); name invented): with the window [0x7ca308] and its list child 0x248e (nothing without them): the kind kept in [0x7ca30c], the list cleared when asked, a row (data the
// index; parent -1) for each valid entry of the staff pool [0x7cf83c] of that type (+0), its texts the name (StaffPoolEntry_FormatName, 0x27) and the wage (Staff_WageForRecord), then the rows refreshed.
void __cdecl Dialog7ca308_FillList(u32 kind, u32 clear);

// 0x49c970 (cdecl (index); called from 0x481555; name invented): with the window [0x7ca308], the staff pool's entry index brought up to date in the list 0x248e (not checked). With a row of that data: the row removed and, while the entry is valid,
// a new row of its name (0x28 characters) and wage, unrefreshed. With none: when the entry is valid and of the kind [0x7ca30c] a new row, then the rows refreshed.
void __cdecl Dialog7ca308_StaffChanged(u32 index);

// 0x49ba00 (cdecl (index); called by 0x49b97b): the staff pool [0x7cf83c]'s entry index (StaffPool_ValidEntry) shown in the window
// [0x7ca308]'s child 0x2484 (Window_FindChild, Window_GetSavedRect), placed as Panel7cc328_ShowStaff places it (the floats [0x6ff984] /
// [0x6ff988], computed every time). Only for an entry whose type +0 / byte +9 differ from [0x7523e8] / [0x7523ec]: those kept, the
// script [0x7ca310] killed when set (SpriteScript_Kill), a new one started (SpriteScript_Start of 0x74f758, anim 1) into [0x7ca310]
// with the folder 5, 6, 4, 7, 8 for the types 0-4 (any other type: the index itself, sic) and the sprite the byte +9, its float
// register 8 set to 8.0 (SpriteScript_SetRegF). EAX not kept.
void __cdecl Dialog7ca308_ShowStaff(u32 index);

// 0x49bdd0 (cdecl (kind); name invented): the staff dialog opened on a kind (-1: the one it had). Window7c24c8_Close, Global7cb28c_Select (1, 2). With the window
// [0x7ca308] already there and another kind: only the kind kept in [0x7ca30c], its radio button (the list 0x248e's child 0x248f), the label 0x2480's text (0x8b + the kind) and the list filled afresh
// (Dialog7ca308_FillList (kind, 1)). Otherwise the window built (the script 0x751fa8, handler 0x49b650, on [0x7cb2ac]; none: nothing more) and made [0x7c24c8]; the line 0x2496 (a TextPart6ff530 as its object +0x98)
// of the money of [0x7cf83c]'s table (Source_Field0c) formatted into 0x7ba058 by the texts 0x1ca / 0x1cb (negative, by its size); when there: the list (its rows' 0x49c870 / 0x49c880, handler 0x49bc80, the sort [0x7523e4], the buttons 0x10 / 0x11 with the font
// [0x7523e0], the label, FillList (kind, 0), the radio button); the bar 0x2481; the figures 0x2485 (the three rings' newest values into [0x7ca2f0], [0x7ca2fc], [0x7ca2f8], the difference [0x7ca300]). Advisor_PostId (0xbd).
void __cdecl Dialog7ca308_Open(u32 kind);

// 0x49bc80 (cdecl (window, message, id, value); the kind radio buttons' handler; name invented): on the message 0x101 with value 1 from a button 0x2490-0x2494 (the jump table 0x49bdbc): the kinds 0, 4, 2, 1, 3: the list filled (Dialog7ca308_FillList (kind, 1)), the label
// 0x2480's text (0x8b + the kind) on the window [0x7ca308], the figure's script [0x7ca310] killed when set and cleared, [0x7523e8] = [0x7523ec] = -1. Always the window's default handler (Window_CallDefault), its result returned.
s32 __cdecl Dialog7ca308_ListHandler(u8 *window, u32 message, u32 id, u32 value);
}
