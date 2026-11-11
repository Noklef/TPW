// The staff list dialog [0x7ca168] (the window of the script 0x750e10 with the handler 0x495da0; its list's handler Dialog7ca168_Handler): its opening. The rest is in dialogs.h. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// Dialog7ca168_Open's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7ca168OpenEnv {
    s32(__cdecl *forEach)(u32 mask, u32 callback);                     // Things_ForEachSelected
    double(__fastcall *average)(void *self, void *, u32 unused);       // Things_AverageStaffStat1f8
    s32(__stdcall *averageOfKind)(u32 kind);                           // Things_AverageStat1f8OfKind
    s32(__fastcall *findItem)(u8 *list, void *, u32 data);             // Window7059f0_FindItemData
    void(__fastcall *scrollTo)(u8 *list, void *, s32 position);        // Window7059f0_ScrollTo
};
void Dialog7ca168_SetOpenEnv(const Dialog7ca168OpenEnv *env);

extern "C" {

// 0x496620 (cdecl (tab, selected); name invented): Window7c24c8_Close, Global7cb28c_Select (2, 4), the window [0x7ca168] built (the script 0x750e10, handler 0x495da0, on [0x7cb2ac]; none: nothing more) and made
// [0x7c24c8]. With its list 0x321 (else on to the panel): its rows (0x496c10 / 0x496d20), five columns, handler 0x4960a0, the sort [0x75118c], the buttons 0x10-0x14 (font [0x751188], the texts 0x65-0x69, values 0x77-0x7b);
// the tab (-1: [0x7ca16c]) kept in [0x7ca16c], the list in [0x7ca164]; tabs 0-4 add a row for each of their staff (Things_ForEachSelected of 2, 4, 0x10, 8, 0x20 with 0x496320), the rows refreshed; a selected data found
// scrolled to; its group 0x322 (when there) of a tab 0-4 given its button. The panel 0x329 (not checked): the text 0x32a none and the bar 0x32d (happygrad, inset 8); the text 0x32b 0x6a and the bar 0x32c, its value the average
// of all the staff (the double * 1024.0f * 0.01f, __ftol); the label 0x32a the tab's text and the bar 0x32d's value the average of the tab's staff * 1024 / 100 (past the tabs the kind is the last allocation, sic: the argument's slot held it).
// The timer 0x80083 of 2000 set and, when that answers (AX), started; Advisor_PostId (0x11d).
void __cdecl Dialog7ca168_Open(u32 tab, u32 selected);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const float, gDialog7ca168_Open_6ff6f4, 0x006ff6f4, 0x4);
GAME_DATA(const float, gDialog7ca168_Open_6ff6f8, 0x006ff6f8, 0x4);
GAME_DATA(u32, gDialog7ca168_Open_751188, 0x00751188, 0x4);
GAME_DATA(u8 *, gDialog7ca168_7ca164, 0x007ca164, 0x4);
GAME_DATA(u8 *, gDialog7ca168_7ca168, 0x007ca168, 0x4);
GAME_DATA(u32, gDialog7ca168_7ca16c, 0x007ca16c, 0x4);

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x374], gDialog7ca168_Open_750e10, 0x00750e10, 0x374);   // a window script (or a text / table before one): to the next named address
