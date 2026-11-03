// The dialog [0x7ca150]'s list's handler, its rows' figures kept current (name invented, unconfirmed): reimplementation of 0x4955e0.
#pragma once

#include "../../../re/types.h"

struct Dialog7ca150ListEnv {
    u32(__fastcall *setTimer)(u8 *window, void *, u32 id, u32 interval);                                       // Window_SetTimer
    u32(__fastcall *startTimer)(u8 *window, void *, u32 id);                                                   // Window_StartTimer
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                                                 // Window7059f0_NthItemData
    void(__fastcall *setTexts)(u8 *window, void *, s32 n, const u32 *texts, u32 mask, u32 refresh, u32 unused);  // Window7059f0_SetItemTexts
    u32(__fastcall *owner)(u8 *object, void *, u32 out);                                                       // Fn_4e0df0
    u8(__fastcall *rating)(const u8 *object, void *, u32 baseOnly);                                            // Object_Rating
    u32(__fastcall *field48)(const u8 *object);                                                                // Object_Field48AsInt
    u32(__fastcall *difference)(const u8 *object);                                                             // Object_HistoryDifference
    u32(__fastcall *diff180)(const u8 *object);                                                                // Obj_Diff180_184
    u32(__fastcall *average)(const u8 *object);                                                                // Object_HistoryAverage
    s32(__fastcall *call)(u8 *window, void *, u32 message, u32 a, u32 b);                                      // Window_CallDefault
};
void Dialog7ca150List_SetEnv(const Dialog7ca150ListEnv *env);

extern "C" {

// 0x4955e0 (cdecl (window, message, a, b)): 0x15: the timer 0x80083 set to 2000 and, if that answers (a word) other than 0, started. 0x10 with a 0x80083, by the tab [0x7ca154], for each row of a window from the last, its item's object o (Window7059f0_SetItemTexts, refreshing at row 0, each text 0 first):
// tab 0: o the table 0x7cfb90's entry of its owner's handle (Fn_4e0df0) if not 0; the mask 0x1e of the sum of the last 30 of its buffer +0x1a8 (truncated), Object_Rating(0)'s byte, +0x44's low byte (truncated) and Object_Field48AsInt's low byte, those three * 1024 / 100. Tab 1: the mask 0x1e of the buffer's
// up-to-30's sum added here, Object_HistoryDifference, Obj_Diff180_184 and Object_HistoryAverage * 1024 / 100. Tab 2: the mask 0x3e of the sum, the rating, the difference, Obj_Diff180_184 and the average. Window_CallDefault's answer.
s32 __cdecl Dialog7ca150_ListHandler(u8 *window, u32 message, u32 a, u32 b);
}
