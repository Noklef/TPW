// The dialog at 0x7cb25c (its mode flag at 0x7cb260: 0 the loans offered, 1 those taken). Reimplementation of 0x49f860, 0x49fa00 and
// 0x4a0770. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// Dialog7cb25c_Open's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cb25cOpenEnv {
    s32(__cdecl *atExit)(void(__cdecl *f)());                                                            // 0x67a7d0
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);                                                 // HoverLabel_Attach
    u32 *(__fastcall *loanOffer)(u8 *money, void *, u32 *out, u32 index);                                // Money_LoanOffer
    void(__fastcall *buildRows)(u8 *window, void *, u8 *(__cdecl *make)(), void(__cdecl *release)(u8 *));   // Window7059f0_BuildRows
    void(__cdecl *fillList)(u8 *list);                                                                   // Dialog7cb25c_FillList
    u16(__fastcall *selectRadio)(u8 *group, void *, u32 id);                                             // RadioGroup_Select
    void(__cdecl *setTexts)();                                                                           // Dialog7cb25c_SetTexts
};
void Dialog7cb25c_SetOpenEnv(const Dialog7cb25cOpenEnv *env);

// Dialog7cb25c_Handler's calls (the ports by default, the unported 0x47f020 raw; the bench's recorders: their own benches have them).
struct Dialog7cb25cHandlerEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                    // Window_SendMessage
    void(__cdecl *cancel)(u32 id);                                                // Game_CancelAdvisor
    u8 *(__fastcall *find)(u8 *window, void *, u32 id);                           // Window_FindChild
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                    // Window7059f0_NthItemData
    u32(__fastcall *table26)(u8 *game);                                           // GameTable7cfb90_Get1da726_5195d0
    u32 *(__fastcall *offer)(u8 *money, void *, u32 *out, u32 index);             // Money_LoanOffer
    u32(__fastcall *field)(u32 source);                                           // Source_Field0c
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);       // Charset_FormatWide
    void(__cdecl *box)(const u8 *record, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g);   // 0x47f020, agent B's, unported
    void(__cdecl *open1)();                                                       // Dialog7ca210_Open
    void(__cdecl *open2)();                                                       // Dialog7ca2e8_Open
    void(__cdecl *open3)();                                                       // Dialog7cc2ec_Open
    s32(__cdecl *keys)(u8 *window, u32 message, u32 a, u32 b);                    // Dialog_KeyHandler
};
void Dialog7cb25c_SetHandlerEnv(const Dialog7cb25cHandlerEnv *env);

extern "C" {

// 0x49f4e0 (cdecl (window, message, a, b); name invented, unconfirmed): 5: the dialog sent 4 (0, 0), answer 1. 0x14: [0x7cb25c] = 0, Game_CancelAdvisor (0xc3), then Dialog_KeyHandler. 0x100: a -2 or -1 the dialog sent 4, 0; 0x12c4ba, 0x12c4bb, 0x12c4bc open Dialog7ca210, Dialog7ca2e8, Dialog7cc2ec, 0; anything else Dialog_KeyHandler.
// 0x400 with the list a there and b not -1: the row's data. Below 0 a loan taken, index -1 - data: owed (the term +0xc less the months paid +0x18) * the rate +0x10; the balance (Source_Field0c, signed) over that the text 0xb5 and a box of two buttons calling 0x49f460 with the index, else the text 0xb6 and a box of one. Otherwise an offer, index data - 1: the text 0xb7 of the amount +4 and a box calling 0x49f4a0. Then Dialog_KeyHandler, as for all else.
s32 __cdecl Dialog7cb25c_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x49fb30 (cdecl; name invented): open already, or the mode setting (0xfb3b7c, read the first time under the guard 0xfb34c2 & 1) 2: nothing. Window7c24c8_Close, Global7cb28c_Select (3, 8), the dialog built (the script
// 0x7525a0, handler 0x49f4e0, on [0x7cb2ac]; none: nothing more), made [0x7c24c8], its label 0x12c4ca the text 0xaa. Its figures 0x12c4c1 (when there): the headings 0x12c4c4 / 0x12c4c2 and in red 0x12c4c9 / 0x12c4c6 (texts
// 0x165, 0x169, 0x168, 0x167); the numbers 0x12c4c5, the newest of the rings +0x1fc94 (A) of the analyser (GameTable7cfb90_Get1da720_519510); 0x12c4c8, the sum of the eight loans' rates +0x10 taken (+0x14; Money_LoanOffer) (B), red;
// 0x12c4c7, the newest of the ring +0x1f5a4 (C) less B, red; 0x12c4c3, A less C. The list 0x12c4bd (not checked): its rows (0x4a02b0 / 0x4a03d0), six columns, handler 0x4a06b0, filled, its buttons 0x10, 0x13, 0x14 (font [0x752938], the
// texts 0xac, 0xaf, 0xb0, values 0xde, 0xe1, 0xe2), 0x11, 0x12, 0x15 no text, -1; the group 0x12c4be given 0x12c4bf when [0x7cb260] is set, else 0x12c4c0. Dialog7cb25c_SetTexts, Advisor_PostId (0xc3).
void __cdecl Dialog7cb25c_Open();

// 0x49f860 (cdecl): with the dialog [0x7cb25c] (nothing without it), its child 0x12c4bd's children 0x11, 0x12 and 0x15 (Window_FindChild,
// not checked) each sent a language text as 0x100 to their impl (Language_WideText; Window_NotifyImpl100) and given a value at +0x90 and
// +0x94: texts 0xae / 0xad / 0xb1 and values 0xdf / 0xe0 / 0xe3 when [0x7cb260] is 0, else texts 0xb2 / 0xb3 / 0xb4 and values 0xe5 / 0xe6 /
// 0xe7; then the dialog's child 0x12c4ca told the text 0xaa (0xab with the flag) through its slot 9.
void __cdecl Dialog7cb25c_SetTexts();

// 0x49fa00 (cdecl (list); name invented): the list (a Window7059f0) cleared, then for each of the eight loans of the world's money record
// (the table 0x7cfb90's entry for [0x7cf83c]'s word +0x1da726; Money_LoanOffer, which marks the offer +0 by the park's credit): with
// [0x7cb260] 0, an offer not taken (+0x14 0) within the credit (+0 set) added with the data i + 1; with it 1, a loan taken added with the data
// -1 - i. The six columns: its text (Records_DataIfTerminated of [0x78bac4] for +0x1c, Language_Widen), the amount +4, the months left
// (the term +0xc, less the months paid +0x18 for a loan taken), +8, the rate +0x10 and the rate times the months left (Window7059f0_AddItem,
// parent -1, refreshed).
void __cdecl Dialog7cb25c_FillList(u8 *list);

// 0x4a0770 (cdecl): with the dialog and [0x7cb260] 1, Dialog7cb25c_FillList on its child 0x12c4bd (Window_FindChild).
void __cdecl Dialog7cb25c_Refresh();

// 0x4a06b0 (cdecl (list, message, a, b); name invented): the list's handler (set at 0x4a0121). On 0x101 with b 1 (a 0x15 test first, of
// no effect): [0x7cb260] = whether a is 0x12c4bf (the button pressed), the list refilled (Dialog7cb25c_FillList) and the texts set
// (Dialog7cb25c_SetTexts). Every message then goes to Window_CallDefault, whose answer is returned.
s32 __cdecl Dialog7cb25c_ListProc(u8 *list, u32 message, u32 a, u32 b);

// 0x49f460 / 0x49f4a0 (cdecl (index); names invented): the callbacks that the dialog's handler (0x49f4e0) hands to a confirmation box
// (pushed at 0x49f646 / 0x49f789). Loan `index` of the world's money record (the table 0x7cfb90's entry for [0x7cf83c]'s word +0x1da726)
// is paid off (Money_PayOffLoan) / taken out (Money_TakeLoan), whatever that answers. Then the dialog's child 0x12c4bd (Window_FindChild
// on [0x7cb25c], unchecked) is refilled (Dialog7cb25c_FillList). Both return 1.
u32 __cdecl Dialog7cb25c_PayOffLoan(u32 index);
u32 __cdecl Dialog7cb25c_TakeLoan(u32 index);

}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gDialog7cb25c_Open_752938, 0x00752938, 0x4);
GAME_DATA(u8 *, gDialog7cb25c_7cb25c, 0x007cb25c, 0x4);
GAME_DATA(u32, gDialog7cb25c_7cb260, 0x007cb260, 0x4);
GAME_DATA(u8, gDialog7cb25c_fb34c2, 0x00fb34c2, 0x1);
