// The dialog [0x7cb400] (ui/dialog7cb400): its handler 0x4a6d00, what it does with the text 0x4a6b80 and its end 0x4a6a50. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

struct WString;

// The calls (the ports by default; 0x5c7f40, 0x5c83b0, 0x5c8a10 and 0x5e2010, agent B's, by address; the bench's recorders: their own benches have them).
struct Dialog7cb400CloseEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                // Window_SendMessage
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                     // Window_SetFocus
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                 // Window_SetShown
    void(__cdecl *enableKeys)();                                              // KeyMaps_EnableAll
    void(__fastcall *clearQueue)(u8 *object);                                 // Class702da0_Clear
    u8 *(__fastcall *readMode)(u8 *setting);                                  // Setting_ReadMode
    s32(__cdecl *atExit)(void(__cdecl *f)());                                 // 0x67a7d0
    u8 *(__cdecl *saveInstance)();                                            // GlobalSave_Instance
    void(__fastcall *saveCount)(u8 *info);                                    // GlobalSave_Inc20
    void(__fastcall *slotsRefresh)(u8 *slots);                                // 0x5c8a10
    void(__fastcall *push)(u8 *object, void *, u32 id, u32 clear);            // Class702da0_Push
    void(__fastcall *pick)(u8 *object, void *, u32 flag);                     // Obj_PickByName
    void(__fastcall *menuRefresh)(u8 *object);                                // 0x5e2010
    void(__cdecl *showTheme)();                                               // Dialog7cc4b4_ShowTheme
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);                // Dialog_BaseHandler
    void(__fastcall *slotsAdd)(u8 *slots, void *, u32 value, WString *text, u32 flag);   // 0x5c7f40
    void(__fastcall *slotsChoose)(u8 *slots, void *, u32 value);              // 0x5c83b0
};
void Dialog7cb400Close_SetEnv(const Dialog7cb400CloseEnv *env);   // nullptr: the game's

extern "C" {

// 0x4a6b80 (cdecl (value), the answer in AL): the buffer 0x7cb3b8's trailing spaces cut. Empty: 0. Otherwise the user slots (UserSlots_Instance, inlined in the original, twice) told the value, the text as a
// wide string and whether the group 0x70c of the dialog [0x7cb400] (Window_FindChild; Fn_66a281) has 0x70d chosen, then the value (0x5c83b0); 1.
u8 __cdecl Dialog7cb400_Commit(u32 value);

// 0x4a6a50 (cdecl (cancelled)): [0x7cb3f8]'s +0x60 unfocused, the message 4 to [0x7cb3f8], the main window ([0xf82884] +4) shown and unfocused, the keys enabled, the scene queue ([0xf85614] +0x14) cleared.
// Unless cancelled: with a slot chosen (the user slots' +0x60 not -1) and [0x7cb3fc] set: mode 2 (the static setting 0xfb3b7c, guard bit 0 of 0xfb34c0) pushes 0x18a, any other the global save's count up,
// 0x5c8a10 on the slots and 0x189 (clear 1 each). Then [0xf85614] +4: Obj_PickByName (1), 0x5e2010; Dialog7cc4b4_ShowTheme.
void __cdecl Dialog7cb400_Close(u32 cancelled);

// 0x4a6d00 (cdecl (window, message, a, b), a window handler). Message 0x14: [0x7cb400] = 0, Dialog_BaseHandler's answer. 0x100 with a -2: the message 4, 0. 0x100 with a -1: Dialog7cb400_Commit of the window's
// word 0 (Window_DwordA0); on 1 [0x7cb3fc] = 1, the message 4, Dialog7cb400_Close (0), 0. 0x802: the message 0x100 (-1, 0), 0. 0x804: the message 4, 0. 0x101 with b 1: the child 0x70b shown when a is 0x70d.
// All else: Dialog_BaseHandler's answer.
s32 __cdecl Dialog7cb400_Handler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8, gDialog7cb400_Close_fb34c0, 0x00fb34c0, 0x1);
