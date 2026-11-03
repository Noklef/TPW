// The UI's one-call entries (menu and key actions, the thing panel's handler) in 0x481550-0x4d0690. The names are invented, unconfirmed. Each calls through MenuActionsEnv so that the bench records it.
#pragma once

#include "../../../re/types.h"

struct Stream;

// Their calls (the ports by default; the bench's recorders: their own benches have them).
struct MenuActionsEnv {
    void(__cdecl *open7cc1f8)(u32 kind, u32 selected);                     // Dialog7cc1f8_Open
    void(__cdecl *open7ca308)(u32 kind);                                   // Dialog7ca308_Open
    void(__cdecl *open7ca150)(s32 tab, u32 selected);                      // Dialog7ca150_Open
    void(__cdecl *open7ca168)(u32 tab, u32 selected);                      // Dialog7ca168_Open
    void(__cdecl *open7ca13c)(u32 selected);                               // Dialog7ca13c_Open
    void(__cdecl *create7cb398)();                                         // Dialog7cb398_Create
    void(__cdecl *staffChanged)(u32 index);                                // Dialog7ca308_StaffChanged
    s32 *(__cdecl *saveTags)(Stream *stream);                              // MsgTags_Save
    void(__cdecl *addRow)(u8 *guest);                                      // Dialog7ca13c_AddRow
    u32(__fastcall *countOfType)(const u8 *pool, void *, u32 type);        // StaffPool_CountOfType
    void(__fastcall *setName)(u8 *staff, void *, const u16 *name);         // Staff_SetName
    u16 *(__fastcall *getName)(u8 *staff);                                 // Staff_GetName
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);             // Window_SendMessage
    u32(__cdecl *child21Post)(u32 value);                                  // Dialog7cb2b8_Child21Post
    u8(__cdecl *postId)(u32 id);                                           // Advisor_PostId
    void(__cdecl *resetChild21)(u32 flag);                                 // Dialog7cb2b8_ResetChild21
    void(__cdecl *cancelAdvisor)(u32 id);                                  // Game_CancelAdvisor
    void(__fastcall *restartRide)(u8 *object);                             // Object_RestartRide
    void(__fastcall *closeObject)(u8 *object);                             // Object_Close
    u32(__fastcall *setFlag64)(u8 *object, void *, u32 on);                // Object_SetFlag64
    u32(__cdecl *setThing)(u32 value);                                     // Global7c2654_Set
    void(__cdecl *focusCell)(u32 record);                                  // FocusCell_Update
    void(__cdecl *setMode)(u32 mode, u32 arm);                             // GameMode_Set
    void(__cdecl *mapClick)(s32 x, s32 y, u32 flags, u32 useStored);       // GameMode_MapClick
    void(__fastcall *clear702da0)(u8 *object);                             // Class702da0_Clear
    void(__fastcall *stepGroup)(u8 *owner);                                // Things_StepGroup
    void(__fastcall *stepBack)(u8 *owner);                                 // Things_StepGroupBack
    s32(__cdecl *keyHandler)(u8 *window, u32 message, u32 a, u32 b);       // Dialog_KeyHandler
    u32(__fastcall *dwordA0)(u8 *window, void *, u32 index);               // Window_DwordA0
};
void MenuActions_SetEnv(const MenuActionsEnv *env);

extern "C" {

// The dialogs' openers with no selection: Dialog7cc1f8_Open (-1, 0; the word 0 on the stack, its upper half the caller's), Dialog7ca308_Open (-1), Dialog7cb398_Create, Dialog7ca150_Open (-1, 0), Dialog7ca168_Open (-1, 0), Dialog7ca13c_Open (0).
void __cdecl Wrap_Dialog7cc1f8_Open_4816e0();
void __cdecl Wrap_Dialog7ca308_Open_481700();
void __cdecl Wrap_Dialog7cb398_Create_481710();
void __cdecl Wrap_Dialog7ca150_Open_481720();
void __cdecl Wrap_Dialog7ca168_Open_481730();
void __cdecl Wrap_Dialog7ca13c_Open_481740();

// 0x481550, 0x4816a0 (stdcall): Dialog7ca308_StaffChanged, MsgTags_Save. 0x4937e0 (cdecl): Dialog7ca13c_AddRow, answers 1. 0x4c9e40 (stdcall): StaffPool_CountOfType of the world [0x80239c].
void __stdcall Wrap_Dialog7ca308_StaffChanged_481550(u32 index);
void __stdcall Wrap_MsgTags_Save_4816a0(Stream *stream);
u32 __cdecl Wrap_Dialog7ca13c_AddRow_4937e0(u8 *guest);
u32 __stdcall World_StaffCountOfType_4c9e40(u32 type);

// 0x4b60d0 (stdcall), 0x4b60f0 (cdecl): Staff_SetName, Staff_GetName of the shown thing [0x7c2658]; none: nothing, 0.
void __stdcall ShownThing_SetName_4b60d0(const u16 *name);
u16 *__cdecl ShownThing_GetName_4b60f0();

// 0x4b8510, 0x4c4470 (cdecl): slot 0xa of [[0xf85614] + 8], slot 0xb of [[0xf85614] + 0x10], no arguments, the answer kept.
u32 __cdecl Game_Part8Slot28_4b8510();
u32 __cdecl Game_Part10Slot2c_4c4470();

// The windows closed: Window_SendMessage ([0x7cb2fc], 4, 0, 0), ([0x7cc8bc], 4, 0, 0), ([0x7cc4c4], 6, 0, 0).
void __cdecl Window7cb2fc_Close_4a4580();
void __cdecl Window7cc8bc_Close_4bf8e0();
void __cdecl Window7cc4c4_Send6_4b9c40();

// 0x4bc640 (cdecl (a, b)): [0x7cc684] = a, [0x7cc65c] = b, EAX a. 0x4b9f60 (stdcall (a, unused)): [0x7cc504] = a, EAX a.
u32 __cdecl Globals7cc684_Set_4bc640(u32 a, u32 b);
u32 __stdcall Global7cc504_Set_4b9f60(u32 a, u32 unused);

// 0x4b4650 (thiscall): +0x24 = 0, +0x28 and +0x2c = -1, answers -1. 0x4d0690 (thiscall): how many of the eight dwords from +0x28, 0x20 apart, are set.
u32 __fastcall Obj_Reset24_4b4650(u8 *object);
u32 __fastcall Obj_Count28s_4d0690(const u8 *object);

// 0x498bb0 (cdecl): Dialog7cb2b8_Child21Post ([0x7ca1e0]; a further 0 pushed), Advisor_PostId (0x131). 0x498bd0 (cdecl): Dialog7cb2b8_ResetChild21 (0), Game_CancelAdvisor (0x131).
void __cdecl Dialog7ca1e0_Post_498bb0();
void __cdecl Dialog7ca1e0_Cancel_498bd0();

// 0x48ccf0 (thiscall (restart), pops 4; this unused): Object_RestartRide of the shown thing, or with 0 Object_Close. 0x48ce50 (thiscall (flag byte), pops 4): Object_SetFlag64 of it (1 when the flag is 0), then slot 5 of this.
void __fastcall ThingPanel_RestartOrClose_48ccf0(u8 *panel, void *, u32 restart);
void __fastcall ThingPanel_SetFlag64_48ce50(u8 *panel, void *, u8 flag);

// 0x48cde0 (cdecl (thing)): Global7c2654_Set (thing), FocusCell_Update (1), GameMode_Set (0x33, 1), GameMode_MapClick of his cell's (column, row; 0, 0): the cell (+7 << 7) + +5 + 1 in 16 bits, less 1, & 0x7f and >> 7. Answers 1.
u32 __cdecl Thing_GoToCell_48cde0(u8 *thing);

// 0x48d200 (cdecl (window, message, a, b)): the panel its dword 6 (Window_DwordA0). 0x14: with a shown thing the panel's slot 0xf; its slots 7, 8, 2, its +0x10 cleared. 0x1000a: a 0x2600 Things_StepGroupBack, 0x2800 Things_StepGroup. Then Dialog_KeyHandler, its answer in EAX.
s32 __cdecl ThingPanel_Handler_48d200(u8 *window, u32 message, u32 a, u32 b);

// 0x4c07d0 (cdecl (show)): Class702da0_Clear of [[0xf85614] + 0x14], Window_SendMessage ([0x7cc984], 4, 0, 0), the callback [0x7cca4c] (cdecl (show, [0x7cca50])); with show Window_SendMessage ([[0xf82884] + 4], 6, 1, 0).
void __cdecl Dialog7cc984_Finish_4c07d0(u32 show);

// 0x48cf90 (cdecl): Thing_NameBuffer7ba058 of the thing [0x7c2658].
u8 *__cdecl ShownThing_NameBuffer();
}
