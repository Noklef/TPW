// The handler of the guests' list [0x7ca13c]. Reimplementation of 0x493230; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/filetime.h"

// The calls (the ports by default; tests/fncmp/dialog7ca13chandler.py gives recorders).
struct Dialog7ca13cHandlerEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                           // Window_SendMessage
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                           // Window7059f0_NthItemData
    u32(__fastcall *timeInPark)(const u8 *visitor);                                      // Visitor_TimeInPark
    FileTime *(__fastcall *span)(const u8 *clock, void *, FileTime *out, u32 ticks);     // FunnyTime_SpanOf
    void(__fastcall *setTexts)(u8 *window, void *, s32 n, const u32 *texts, u32 mask, u32 refresh, u32 unknown);   // Window7059f0_SetItemTexts
    u16 *(__cdecl *text)(u32 index);                                                     // Language_WideText
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);                                 // HoverLabel_Attach
    void(__cdecl *cancelAdvisor)(u32 id);                                                // Game_CancelAdvisor
    void(__cdecl *create7cb398)();                                                       // Dialog7cb398_Create
    void(__cdecl *open7ca168)(u32 tab, u32 selected);                                    // Dialog7ca168_Open
    void(__cdecl *open7ca150)(s32 tab, u32 selected);                                    // Dialog7ca150_Open
    u16 *(__fastcall *handle)(const u8 *thing, void *, u16 *out);                        // Thing_GetHandle
    void(__cdecl *camera)(u32 handle);                                                   // Camera_CentreOnThing
    void(__fastcall *show)(u8 *self, void *, u8 *thing);                                 // Panel7cc408_Show
    s32(__cdecl *keys)(u8 *window, u32 message, u32 a, u32 b);                           // Dialog_KeyHandler
};
void Dialog7ca13cHandler_SetEnv(const Dialog7ca13cHandlerEnv *env);   // nullptr: the game's

extern "C" {

// 0x493230 (cdecl (window, message, a, b)). 5: [0x7ca13c] sent (4, 0, 0), answer 1. 0x10 (the timer a 0x80083, [0x7ca13c] there): each row of its list 0x1e497 from the last set again from its guest (as Dialog7ca13c_AddRow, the time in park << 2; mask 0x3f, row 0 refreshing, -1). 0x14: [0x7ca13c] = 0, Game_CancelAdvisor(0x11e). 0x15: HoverLabel_Attach(window, 0x1e498, text 0x70).
// 0x100: a -2: [0x7ca13c] sent (4, 0, 0); 0x1e499: Dialog7cb398_Create; 0x1e49a: Dialog7ca168_Open(-1, 0); 0x1e49b: Dialog7ca150_Open(-1, 0): answer 0. 0x400: child a there and b not -1: Panel7cc408_Show on 0x7cc408 with row b's data; answer 0 anyway. 0x402: child a there and b not -1: Camera_CentreOnThing on row b's data's handle, [0x7ca13c] sent (4, 0, 0), answer 0. 0x406: [0x7508bc] = b.
// All the rest: Dialog_KeyHandler, its answer.
s32 __cdecl Dialog7ca13c_Handler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(s32, gDialog7ca13c_7508bc, 0x007508bc, 0x4);
