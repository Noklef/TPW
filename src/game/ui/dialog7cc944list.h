// The dialog [0x7cc944]'s list filled from an online reply's records. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/stdwstring.h"
#include "../core/string.h"

extern "C" {

// What the fill calls (the benches point these at recorders): the ports, 0x5d3230 and 0x5b50b0 by address (agent B's).
struct Dialog7cc944ListEnv {
    void(__fastcall *clear)(u8 *window);                                                   // Window7059f0_Clear 0x6649d5
    const u16 *(__cdecl *wideText)(u32 id);                                                // Language_WideText 0x485b00
    s32(__fastcall *addItem)(u8 *window, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);   // Window7059f0_AddItem 0x66403b
    u8 *(__fastcall *saveCtor)(u8 *block);                                                 // GlobalSave_Construct 0x5aef50
    u8(__fastcall *saveByte25)(const u8 *save);                                            // GlobalSave_Byte25 0x5b09b0
    void(__fastcall *filterText)(u8 *save, void *, StdWString *text, u32 flag);            // 0x5d3230 (agent B's)
    u8 *(__fastcall *onlineCtor)(u8 *block);                                               // 0x5b50b0 (agent B's)
    u32(__fastcall *valueByKey)(u8 *owner, void *, u32 key);                               // Owner2c_ValueByKey 0x5bd270
    void(__fastcall *themeLabel)(u8 *self, void *, const String *theme, u8 *label);        // NamedObjects_SetThemeLabelAnsi 0x4099b0
    u8 *(__fastcall *findByKey)(u8 *owner, void *, u32 key);                               // Owner2c_FindByKey 0x5bc9c0
    u16(__fastcall *setState)(u8 *window, void *, u8 state, u8 notify);                    // Window705a50_SetState 0x668bec
};
// The fill's calls go to env (nullptr: the defaults).
void Dialog7cc944List_SetEnv(const Dialog7cc944ListEnv *env);

// 0x4bfc70 (cdecl (reply); name invented, unconfirmed): nothing without [0x7cc944]. Its list 0xe05b cleared; a reply with no records (+0xc) gets one row of the text 0x133. Else a row
// per node of the list +4 (the record +0, the next +4): the record's wide text +0x14 in a string, filtered (0x5d3230 (1)) when the save's byte 25 says so (the save 0xf7c58c made if
// none); the theme's label (the value of the record's +0x4c in the online holder 0xf7d88c's owner +0xf8, NamedObjects_SetThemeLabelAnsi) in a WString; the entry of the record's +0x48 in
// its owner +0xe0, that entry's +0x14; the record's +0xb8, +0x50, +0x54 (Window7059f0_AddItem with the record as its data, refreshing on the last). Then the child 0xe059's state 0 notifying.
void __cdecl Dialog7cc944_FillList(const u8 *reply);
}
