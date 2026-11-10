// The staff list dialog [0x7ca168]'s handler (name invented, unconfirmed): reimplementation of 0x495da0.
#pragma once

#include "../../../re/types.h"

struct Dialog7ca168HandlerEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                                                 // Window_SendMessage
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                                                 // Window7059f0_NthItemData
    u32(__fastcall *mode)(const u8 *object);                                                                   // Mode19c_ToIndex
    u32(__fastcall *hasRegion)(const u8 *staff);                                                               // Staff_HasPatrolRegion
    u8 *(__fastcall *record)(const u8 *object, void *, u32 index);                                             // Records_DataIfTerminated
    u16 *(__cdecl *widen)(const u8 *text);                                                                     // Language_Widen
    s32(__fastcall *wage)(const u8 *staff);                                                                    // Staff_GetWage
    u32(__fastcall *grade)(const u8 *staff);                                                                   // Staff_GradeLevel
    void(__fastcall *setTexts)(u8 *window, void *, s32 n, const u32 *texts, u32 mask, u32 refresh, u32 unused);  // Window7059f0_SetItemTexts
    s32(__cdecl *key)(u8 *window, u32 message, u32 a, u32 b);                                                  // Dialog_KeyHandler
    u16 *(__cdecl *wideText)(u32 index);                                                                       // Language_WideText
    void(__cdecl *hover)(u8 *parent, u32 id, u32 text);                                                        // HoverLabel_Attach
    void(__cdecl *cancel)(u32 id);                                                                             // Game_CancelAdvisor
    void(__cdecl *hire)();                                                                                     // Dialog7cb398_Create
    void(__cdecl *jobs)(s32 tab, u32 selected);                                                                // Dialog7ca150_Open
    void(__cdecl *list)(u32 selected);                                                                         // Dialog7ca13c_Open
    u16 *(__fastcall *handle)(const u8 *thing, void *, u16 *out);                                              // Thing_GetHandle
    void(__cdecl *centre)(u32 handle);                                                                         // Camera_CentreOnThing
    void(__fastcall *show)(u8 *self, void *, u8 *thing);                                                       // Panel7cc3b0_Show
};
void Dialog7ca168Handler_SetEnv(const Dialog7ca168HandlerEnv *env);

extern "C" {

// 0x495da0 (cdecl (window, message, a, b)): 5: the dialog [0x7ca168] sent (4, 0, 0), 1. 0x10 with a 0x80083 and a dialog: each row of its list 0x321 from the last: the item's staff, the cells of the mask 0x1a (Window7059f0_SetItemTexts, refreshing at row 0): its mode's record text
// (mode 1 without a patrol region as 0), its grade * 1024 / 100 (unsigned) and its happiness +0x1f8's low byte (truncated) * 1024 / 100. 0x14: [0x7ca168] = 0, Game_CancelAdvisor(0x11d). 0x15: the label 0x328 of text 0x64. 0x100: a -2: the dialog sent (4, 0, 0); 0x32e: Dialog7cb398_Create; 0x32f:
// Dialog7ca13c_Open(0); 0x330: Dialog7ca150_Open(-1, 0): 0 for these. 0x400: the child a's item b's data (a child, b not -1) shown (Panel7cc3b0_Show on 0x7cc3b0); 0. 0x402: likewise (a child, b not -1) the camera centred on its thing and the dialog sent (4, 0, 0); 0. 0x406: [0x75118c] = b.
// All else: Dialog_KeyHandler's answer.
s32 __cdecl Dialog7ca168_WindowHandler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(s32, gDialog7ca168_75118c, 0x0075118c, 0x4);
