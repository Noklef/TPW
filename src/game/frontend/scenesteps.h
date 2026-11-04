// Front-end scene objects' buttons (slots of the vtables 0x702ec0 and 0x702dd0): a step to the next or previous item, a choice closed, a card opened. Thiscall; names invented, unconfirmed.
// The state at +0x14 is 0 when idle, 2 with a model to play at [+0xc]'s +8.
#pragma once

#include "../../../re/types.h"

// Their calls go to env (nullptr: the ports).
struct SceneStepsEnv {
    u32(__fastcall *play)(const u8 *model, void *, u32 entry, u32 now);   // LobbyModel_Play (0x5d83f0)
    u32(__cdecl *showChild)();                                            // FrontEnd_Dialog7cc4b4Child (0x4b8ea0)
    void(__cdecl *showTheme)();                                           // Dialog7cc4b4_ShowTheme (0x4b90f0)
    u8 *(__fastcall *readMode)(u8 *setting);                              // Setting_ReadMode (0x550ca0)
    s32(__cdecl *atExit)(void(__cdecl *fn)());                            // crt_atexit, raw (0x67a7d0)
    void(__fastcall *next)(u8 *self);                                     // Class702cb0_NextItem (0x5e1640)
    void(__fastcall *previous)(u8 *self);                                 // Class702cb0_PreviousItem (0x5e1730)
    void(__fastcall *resetConnections)(u8 *session);                      // Online_ResetConnections (0x5b6070)
    s32(__cdecl *dialogOn)();                                             // Dialog7cc47c_Message6On (0x4b8810)
    s32(__cdecl *dialogOff)();                                            // Dialog7cc47c_Message6Off (0x4b8830)
    void(__cdecl *open)(const u8 *record, u8 *parent, u32 callback, u32 a, u32 b);   // Dialog7cc984_Open (0x4c0ae0)
    void(__fastcall *setMode)(u8 *setting, void *, u32 mode);             // Setting_SetMode (0x550d80)
    void(__fastcall *step)(u8 *self);                                     // Class702cb0_Step (0x5e0470)
    void(__cdecl *showRecord)(const u8 *holder);                          // Dialog7cc47cInfo_Show (0x4b8850)
    u8 *(__cdecl *users)();                                               // UserSlots_Instance (0x48bcd0)
    void(__fastcall *queueShow)(u8 *queue, void *, u32 id, u32 time);     // the queue's show (0x5d61a0)
};
void SceneSteps_SetEnv(const SceneStepsEnv *env);

extern "C" {

// 0x5e1890 (0x702ec0 slot 6): a state not 0 ended (2: the model played (1, 0)), +0x14 = 0; then the dialog 0x7cc4b4's child and theme shown. 1 when a state ended, else 0.
u32 __fastcall Scene702ec0_EndState(u8 *self);

// 0x5e1ee0 / 0x5e1f40: when idle and the mode setting 0xfb3b7c (read the first time, guard 0xfb2758 bit 0) is not 2: Class702cb0_NextItem / _PreviousItem, the child and theme shown.
void __fastcall Scene702ec0_Next(u8 *self);
void __fastcall Scene702ec0_Previous(u8 *self);

// 0x5e2360 (0x702dd0 slot 6): a state not 0 ended as 0x5e1890. A state ended or the session's +0x384 set: the connections reset and 1 returned, else 0; either way Dialog7cc47c_Message6On.
u32 __fastcall Scene702dd0_EndState(u8 *self);

// 0x5e2770 / 0x5e2790 (0x702dd0): when idle: Class702cb0_NextItem / _PreviousItem, +0xe8 = 1, Dialog7cc47c_Message6On.
void __fastcall Scene702dd0_Next(u8 *self);
void __fastcall Scene702dd0_Previous(u8 *self);

// 0x5e22d0 (0x702dd0): Class702cb0_Step; then with +0xe4, +0xe8 set and a record at [+0xc]'s +0x70: 0x4b8850 of the holder.
void __fastcall Scene702dd0_Step(u8 *self);

// 0x5e1830 (0x702ec0): Class702cb0_Step; then with a user slot chosen (UserSlots_Instance's +0x60 not -1) the queue [[0xf85614] + 0x14] shown the id 0x18b or, by rand()'s low bit clear, 0x18a, for 0x15f90.
void __fastcall Scene702ec0_Step(u8 *self);

// 0x5dd610 (0x702e70 slot 6): with the session's +0x384 set: the connections reset, slot 17 (+0x44) called, 1; else 0.
u32 __fastcall ViewCtl702e70_Back(u8 *self);

// 0x5dcfc0 (cdecl; the callback ViewCtl702bc8_Enter hands ListRequest_Start30): the object's slot 11 when failed, else its slot 10 given the list. Its result.
u32 __cdecl ViewCtl702bc8_ListDone(u32 failed, u8 *object, u32 list);

// 0x5e26a0 (0x702dd0 slot 17): when idle with a record at [+0xc]'s +0x70: +0xe8 = 0, Dialog7cc47c_Message6Off, Dialog7cc984_Open(it, 0, 0x5e2650, 0x5e2660, this).
void __fastcall Scene702dd0_OpenRecord(u8 *self);

// 0x5e2700: [0xf82884]'s +0x14 = 2 and the mode setting (read the first time, as above) set to 1.
void __cdecl Scene_SetOnlineMode();
}
