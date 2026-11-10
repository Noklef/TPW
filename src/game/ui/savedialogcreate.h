// The save and load dialogs' making (names invented, unconfirmed): reimplementation of 0x49efb0, 0x49f280.
#pragma once

#include "../../../re/types.h"

struct SaveDialogCreateEnv {
    void(__fastcall *disable)(u8 *controls, void *, u32 a, u32 b);                                       // 0x4092a0
    u32(__cdecl *closeOther)();                                                                          // Window7c24c8_Close
    u8 *(__cdecl *create)(u8 *script, u32 handler, u32 shaded);                                          // Dialog_Create
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                                                // Window_SetFocus
    u16 *(__cdecl *wideText)(u32 index);                                                                 // Language_WideText
    void(__cdecl *hover)(u8 *parent, u32 id, u32 text);                                                  // HoverLabel_Attach
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                                            // Window_SetShown
    void(__fastcall *buildRows)(u8 *window, void *, u8 *(__cdecl *make)(), void(__cdecl *release)(u8 *));  // Window7059f0_BuildRows
    void(__fastcall *setColumn)(u8 *window, void *, u32 column, u32 value);                              // Window7059f0_SetColumnBottom
    u32(__cdecl *findByName)(const char *name);                                                          // GuiWindows_FindByName
    void(__fastcall *setHighlight)(u8 *window, void *, u8 *highlight);                                   // Window7059f0_SetHighlight
    void(__cdecl *fillLoad)(u8 *window);                                                                 // LoadDialog_FillList
    void(__cdecl *fillSave)(u8 *window);                                                                 // SaveDialog_FillList
    u8 *(__fastcall *part)(u8 *part, void *, u32 value);                                                 // TextPart6ff4d0_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                                             // Window_SetImpl
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                                             // Window_Set4c
    u8 *(__cdecl *font)(u32 index);                                                                      // Language_Font
    u32(__fastcall *setBuffer)(u8 *window, void *, u32 buffer, u32 size);                                // Window705a20_SetBuffer
    void(__fastcall *setCaret)(u8 *window, void *, u8 *caret);                                           // Window705a20_SetCaret
    u32(__fastcall *setRefused)(u8 *window, void *, u32 text);                                           // Fn_66781a
    void(__cdecl *keymaps)();                                                                            // KeyMaps_ResetAllAndSetFlag
};
void SaveDialogCreate_SetEnv(const SaveDialogCreateEnv *env);

extern "C" {

// 0x49efb0 (cdecl): the controls off (0x4092a0 (0, 0), 0x786ba4 = 0), Window7c24c8_Close, the dialog from the script 0x7523f0 with LoadDialog_Handler shaded in [0x7cb244]; if made, focus 0, the label 0x23bce1 of text 0xca, the children -1 and 0x23bce2 hidden; the list 0x23bce0, if any: its rows
// (List7cb24c_NewRow, _InitRow), its two columns' bottoms 0, the "hilight" window its highlight, LoadDialog_FillList.
void __cdecl LoadDialog_Create();
// 0x49f280 (cdecl): the same with SaveDialog_Handler in [0x7cb248] and text 0xc9; the child 0x23bce2 the edit box: a TextPart6ff4d0 (none if the allocation fails) as its impl, font 7, the buffer 0x7ca720 of 0xf characters, the "hilight" window its caret, its text 0xce (slot 9), the
// refused characters 0x752588 and focus 0. The list as for the load dialog with SaveDialog_FillList, then KeyMaps_ResetAllAndSetFlag.
void __cdecl SaveDialog_Create();
}

#include "../core/gamedata.h"

// Globals by address (a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x174], gSavedialogcreate_7523f0, 0x007523f0, 0x174);   // the load and save dialogs' window script
GAME_DATA(u16[10], gText_Savedialogcreate_752588, 0x00752588, 0x14);   // L"\\/*?:|<>\"", the edit box's refused characters
