// The save and load dialogs' handlers and list fillers (the list of saved games at 0xf7b560; names invented, unconfirmed): reimplementation of 0x49e880, 0x49e9b0, 0x49ea30, 0x49ec80, 0x49edf0.
#pragma once

#include "../../../re/types.h"

struct WString;

struct SaveDialogEnv {
    u8 *(__fastcall *first)(u8 *list);                                      // 0x5ac8f0
    void(__fastcall *save)(u8 *list, void *, const WString *name);          // 0x5ac610
    void(__fastcall *load)(u8 *list, void *, u8 *item);                     // 0x5ac5d0
    void(__fastcall *enable)(u8 *controls);                                 // 0x409300
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);              // Window_SendMessage
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);              // Window_PostMessage
    u32(__cdecl *restore)();                                                // Focus_Restore
    void(__cdecl *keymaps)();                                               // KeyMaps_EnableAllAndClearFlag
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);              // Dialog_BaseHandler
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);              // Window7059f0_NthItemData
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);  // Charset_FormatWide
    void(__cdecl *box)(const u8 *record, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g);  // 0x47f020
    s32(__fastcall *addItem)(u8 *window, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);  // Window7059f0_AddItem
};
void SaveDialog_SetEnv(const SaveDialogEnv *env);

extern "C" {

// 0x49e880 (the handler of the load dialog [0x7cb244]): 5 closes it (message 4) and answers 1; 0x14 clears the global, restores the focus and enables the controls (0x786ba4 = 1, 0x409300) before the base handler; 0x100 with a = -2 closes it and answers 0;
// 0x400 from a child (a) with a row (b not -1) loads the row's item (0x5ac5d0), closes it and answers 0 (the row is the item's index: the list is walked that far, none for a negative one). The rest: Dialog_BaseHandler.
s32 __cdecl LoadDialog_Handler(u8 *window, u32 message, u32 a, u32 b);
// 0x49e9b0 (the box's answer; unused): saves under the typed name 0x7ca720 (0x5ac610) and closes the save dialog [0x7cb248]. Answers 1.
u32 __cdecl SaveDialog_Save(u32 unused);
// 0x49ea30 (the handler of the save dialog [0x7cb248]): 5 and 0x14 as for the load dialog (0x14 also KeyMaps_EnableAllAndClearFlag); 0x100: a = -2 closes it and answers 0; a = -1 saves at once (SaveDialog_Save) when the typed name is no item's, answering 0,
// else asks first, a box (text 0xcd with that name, its answer SaveDialog_Save) before the base handler; 0x400 from a child with a row puts the row's item's name into the child 0x23bce2 (slot 9); 0x802 and 0x804 post themselves 0x100 with -1 and -2.
s32 __cdecl SaveDialog_Handler(u8 *window, u32 message, u32 a, u32 b);
// 0x49ec80 (cdecl (the list window)): a row for each item: its name, and its time (FileTimeToLocalFileTime of the item's +0x14) as text 0x1c0 (day, month, year) followed by "hour:minute" in 0x7ba058. The row's data is its index.
void __cdecl LoadDialog_FillList(u8 *window);
// 0x49edf0 (cdecl (the list window)): the same, with its format arguments on the stack in another order and an unused WString.
void __cdecl SaveDialog_FillList(u8 *window);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[72], gSavedialog_7ca720, 0x007ca720, 0x48);
GAME_DATA(u32, gSavedialog_7cb244, 0x007cb244, 0x4);
GAME_DATA(u32, gSavedialog_7cb248, 0x007cb248, 0x4);
