// The dialog window at 0x7cc1f8 (its handler's switch calls the dresser on message 0x15). Reimplementation of 0x4ab6b0, 0x4abfb0 and 0x4abc80. The names are
// unconfirmed.
#pragma once

#include "../../../re/types.h"

// Dialog7cc1f8_FillList's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cc1f8Env {
    void(__fastcall *clear)(u8 *list);                                                        // Window7059f0_Clear
    s32(__fastcall *addItem)(u8 *list, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);   // Window7059f0_AddItem
    void(__fastcall *refresh)(u8 *list);                                                      // Window7059f0_RefreshRows
    void(__fastcall *fullName)(u8 *defs, void *, u32 id, u16 *out, u32 size);                 // ObjectDefs_GetFullName
    u16 *(__cdecl *text)(u32 index);                                                          // Language_WideText
};
void Dialog7cc1f8_SetEnv(const Dialog7cc1f8Env *env);

// Dialog7cc1f8_TabHandler's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cc1f8TabEnv {
    void(__cdecl *showStats)(u32 mode);                                     // Dialog7cc1f8_ShowStats
    void(__cdecl *fillList)(u32 kind, u32 clear);                           // Dialog7cc1f8_FillList
    u16 *(__cdecl *text)(u32 index);                                        // Language_WideText
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);                    // HoverLabel_Attach
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
};
void Dialog7cc1f8_SetTabEnv(const Dialog7cc1f8TabEnv *env);

// Dialog7cc1f8_Open's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cc1f8OpenEnv {
    void(__cdecl *fillList)(u32 kind, u32 clear);                                   // Dialog7cc1f8_FillList
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);         // Charset_FormatWide
    u16(__fastcall *selectRadio)(u8 *group, void *, u32 id);                        // RadioGroup_Select
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);                            // HoverLabel_Attach
    s32(__fastcall *findItem)(u8 *list, void *, u32 data);                          // Window7059f0_FindItemData
    void(__fastcall *scrollTo)(u8 *list, void *, s32 position);                     // Window7059f0_ScrollTo
    void(__fastcall *buildRows)(u8 *list, void *, u8 *(__cdecl *make)(), void(__cdecl *release)(u8 *));   // Window7059f0_BuildRows
};
void Dialog7cc1f8_SetOpenEnv(const Dialog7cc1f8OpenEnv *env);

extern "C" {

// 0x4aaf70 (cdecl (kind, clear); name invented): the dialog [0x7cc1f8]'s list 0x1f8 (no dialog or list: nothing; the store GameTable7cfb90_Get1da720_519510 asked first) cleared when clear is set, the
// kind to [0x7cc1fc]. For each object definition (ObjectDefs_First / Next of 0x7890a0) with +0x508 0, +0x4ac the kind, +0x284 0 and its control record's (ObjectControls_Find) byte +0x10 set, a row
// (AddItem with data the id, parent -1, no refresh): a positive price +0xc4 without the key (GlobalSave_HasKey): the text 0x89 and the price negated; else the full name (ObjectDefs_GetFullName, 0x42
// units on the stack) and the control's +4; its state 1 with ObjectControls_GetWord18 positive, else 2 when the id is one of the kind's three latest (Ring16_Get of the store), else 0. Kind 3 adds the
// rows of data -1 (text 0x86, the dword [0x78560c], state 0) and -2 (text 0x87, 0, 0). Last Window7059f0_RefreshRows.
void __cdecl Dialog7cc1f8_FillList(u32 kind, u32 clear);

// 0x4aca60 (cdecl (window, message, a, b); the tabs' handler, name invented): message 0x101 with b 1 and a 0x1fa, 0x1fb, 0x1fc or 0x1fd picks the kind 2, 0, 3 or 1: Dialog7cc1f8_ShowStats (kind), Dialog7cc1f8_FillList (kind, 1), the dialog's
// child 0x1fe labelled with the text 0x77 + kind. Always then the default handler, its result.
u32 __cdecl Dialog7cc1f8_TabHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x4acb70 (cdecl (window, message, a, b); name invented): a list row's handler. Message 0x403 colours the row for the item b (Window_Notify40000) before the default handler: a word below 0 (e7 ef 4b ff); an object
// with a price +0xc4 and no key (GlobalSave_HasKey): ff, e0 or 0 by ObjectDef_IsUnlocked, 0, ff; else by its cost +4 against the balance: above it ff 0 0 ff, else white.
s32 __cdecl Dialog7cc1f8_RowHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x4acc70 (cdecl (kind, selected: its low word); name invented): Global7cb28c_Select (1, 1), the kind -1 taken from [0x7cc1fc]. An open dialog [0x7cc1f8]: its list refilled (clear 1), then, with the child 0x1f8
// (none: nothing more), the kind's button in its group 0x1f9 (kinds 0-3: 0x1fb, 0x1fd, 0x1fa, 0x1fc) and a non-zero selected data found scrolled to. A new one: Dialog7cb2b8_ResetChild21 (1), Window7c24c8_Close, [0x7cc1f0], the
// words [0x7cc1e4], [0x7cc1e8], [0x7cc1e0] 0, built from the script 0x754cf8 (handler 0x4ac270 by address; none: only Advisor_PostId) as [0x7c24c8]; the money line 0x200 (a number argument 13 by its size, the texts 0x1ca / 0x1cb) on
// an 0x6ff530 part; in the child 0x1ea: 0x1eb a RendererPart_Ctor part (0xd8 bytes), 0x1ec a TextPart_Ctor part without text, both raised by 0xc; the list 0x1f8 (none: only Advisor_PostId): handler 0x4aca60, rows
// 0x4ad240 / 0x4ad340, three columns, the hilight, the sort [0x755140], the buttons (font [0x755138]) 0x10, 0x11, 0x12 with the texts 0x7b, 0x7c, 0x8a and values 0x90-0x92; the kind's button, the list filled (clear 0), the label
// 0x1fe of the kinds 0-3 the text 0x77 + kind, the selected scrolled to; Advisor_PostId (0xbc).
void __cdecl Dialog7cc1f8_Open(u32 kind, u32 selected);

// 0x4ab6b0 (no arguments): the dialog's child 0x1ed (Window_FindChild; nothing when it has none) gets its children dressed in this order
// (ui/panelparts; a text a TextPart_Ctor part with that language text, a bar's value given to the child's slot 7): 0x1f4 text 0x7d, 0x1f3
// text 0x7e, 0x1f1 text 0x7f, 0x1f6 text 0x80, 0x1f7 text 0x81, 0x1f5 text 0x88, 0x1f2 text 0x88 (sic, the same text), 0x1ee a
// ridestatbar bar with 0x1f4, 0x1ef one with 0xc8, 0x1f0 one with 0x64.
void __cdecl Dialog7cc1f8_DressStats();

// 0x4abfb0 (cdecl (mode)): the dialog's child 0x1ed (nothing when it has none, the mode looked at after) gets some of those children
// shown or hidden (Window_FindChild, Window_SetShown with 1 or 0; a missing one skipped), in the order 0x1f3, 0x1f1, 0x1f6, 0x1f7, 0x1ee,
// 0x1f2, 0x1ef, 0x1f0: mode 0 all shown; 1 and 3 all hidden; 2 0x1f3 and 0x1ee shown, the others hidden (0x1f6 twice, sic); others nothing.
void __cdecl Dialog7cc1f8_ShowStats(u32 mode);

// 0x4abc80 (cdecl (item: its id word); called by 0x4ac478; name invented): the help tip of a build list item under the mouse. With the
// mouse (Gui_GetMousePos) outside the dialog's child 0x1f8's saved rectangle (Window_FindChild, not checked; Window_GetSavedRect into
// a GuiRect) the tip state [0x75513c] -1. Else for a negative id other than the one tipped (the word [0x7cc1e0]): the state 4, the id
// tipped and the help background given (HelpBg_SetText) the text 0x21a for -1, 0x21b for -2 (the table [0x78ba98], 0x80 characters;
// for another the stack buffer as it is, sic). For an object id: the state 0 / 1 when its definition's +0xc4 is set and the global
// save lacks the key (GlobalSave_HasKey): 0 unlocked (ObjectDef_IsUnlocked), 1 not; else 3 when its control record's cost +4
// (ObjectControls_Find on the game [0x7cf83c]'s +0x1d943c, quiet; read first) is at most the money record's balance
// (Source_Field0c of GameTable7cfb90_Get1da726_5195d0), 2 when not. With the state or the id new both stored and the tip given the text
// 0x97 / 0x98, or 0x96 / 0x94 with the definition's name (its text +0x90 of the table [0x78bad0], 0x3f characters) as the string
// argument 0.
void __cdecl Dialog7cc1f8_ShowItemTip(const s16 *item);
}
