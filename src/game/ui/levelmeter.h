// A GUI window that shows a level with an animated light (its child 0xd2d06; the callers about 0x4a9cc4 set four of them). Its state
// lives in the window's dwords +0xa0 (Window_DwordA0): 0 the light's position (16.16), 1 the clock when last moved, 2 the speed, 3 the
// phase (0-3). Reimplementation of 0x4a95b0, 0x4a9fc0, 0x4a9c70, 0x4a9bd0, 0x4aa120, 0x4aaec0 and 0x481520. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// ResearchPanel_Handler's calls (the ports by default; the bench's recorders: their own benches have them).
struct ResearchPanelEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);         // Window_SendMessage
    void(__cdecl *showProgress)(u32 group);                            // ResearchMeters_ShowProgress
    void(__cdecl *update)(s32 group);                                  // ResearchMeters_Update
    u32(__fastcall *setTimer)(u8 *window, void *, u32 id, u32 interval);   // Window_SetTimer
    u32(__fastcall *startTimer)(u8 *window, void *, u32 id);           // Window_StartTimer
    u16 *(__cdecl *text)(u32 index);                                   // Language_WideText
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);               // HoverLabel_Attach
    void(__cdecl *forward6)(u32 value);                                // Sound_Forward6
    void(__cdecl *cancel)(u32 id);                                     // Game_CancelAdvisor
    u32(__fastcall *table)(u8 *game);                                  // GameTable7cfb90_Get1da720_519510
    u32(__cdecl *sum)();                                               // Things_SumChain
    u32(__fastcall *lab)(u8 *game);                                    // GameTable7cfb90_Get1da722_519550
    u8 *(__fastcall *child)(u8 *window, void *, u32 id);               // Window_FindChild
    u32(__fastcall *setChannel)(u8 *object, void *, u32 channel, u32 volume);   // Volumes_SetChannel
    void(__fastcall *setMaster)(u8 *object, void *, u32 volume);       // Volumes_SetMaster
    void(__fastcall *notify)(u8 *window, void *, u32 value);           // Window_Notify1
    u32(__cdecl *setParam)(u32 sound, u32 parameter, u32 value);       // Sound_SetParam
    void(__cdecl *stop)(u32 id);                                       // Sound_Stop
    u32(__cdecl *play)(u32 sound);                                     // SoundUi_Play
    s32(__cdecl *keys)(u8 *window, u32 message, u32 a, u32 b);         // Dialog_KeyHandler
};
void ResearchPanel_SetEnv(const ResearchPanelEnv *env);

extern "C" {

// 0x4a95b0 (cdecl (window, level); name unconfirmed): level 0 stops it: in phase 3 or 1 the phase becomes 2, the light told 0x20
// (Window_Notify1) and the position 0x200000 (any other phase: nothing). Any other level starts it: the speed ((level + 0x14) << 16) / 100
// (signed); in phase 0 or 2 first the clock (GuiClock_Now), the phase 3, the position 0x300000 and the light told 0x30.
void __cdecl LevelMeter_SetLevel(u8 *window, u32 level);

// 0x4a9fc0 (cdecl (window, message, a, b); name unconfirmed): the window's handler. On message 0x1e in a phase other than 0 the
// position moves on by twice the speed (dword 1 read, not used): phase 1 wraps to 0 past 0x1f (in 16.16); phase 2 past 0x2f turns to
// phase 0 at 0x30; phase 3 past 0x3f turns to phase 1 at 0. The light is told the position's integer part (in any other phase, 0x1e), the
// position and the clock (GuiClock_Now) are kept. Every message then goes to Window_CallDefault (its result returned).
s32 __cdecl LevelMeter_OnMessage(u8 *window, u32 message, u32 a, u32 b);

// 0x4a9c70 (cdecl (group); name unconfirmed): the research panel [0x7cc1d8]'s meters. A group's level is 0 without a current project
// (ResearchLab_CurrentObjId's id 0), else the lab's Table_ScaledBySum of the group; the lab the game [0x7cf83c]'s, found again for each.
// Group -1: the groups 0, 4, 1, 2, 3 each shown on its meter (0xd2d01, 0xd2d07, 0xd2d08, 0xd2d09, 0xd2d0a: Window_FindChild, not checked;
// LevelMeter_SetLevel); any other group's level taken and, for 0-4, shown. Then the five levels again (0, 4, 1, 2, 3), their sum / 5
// (signed) the sound [0x7cc1d0]'s parameter 0x12 (Sound_SetParam). The original's ids land in its own argument's slot (sic, unseen).
void __cdecl ResearchMeters_Update(s32 group);

// 0x4a9bd0 (cdecl (group); name unconfirmed): the group's research left (ResearchLab_LeftPercent of the game [0x7cf83c]'s lab) taken
// from 100; for groups 0-4 (unsigned) that meter's (as ResearchMeters_Update's: Window_FindChild, not checked) child 0xd2d05 (not checked)
// given (done << 10) / 100 (signed) through its slot 7.
void __cdecl ResearchMeters_ShowProgress(u32 group);

// 0x4aa120 (cdecl (group); name unconfirmed): the group's meter (groups 0-4 as ResearchMeters_Update's; any other none, a fault) child 0xd2d05
// (not checked) told (Window_NotifyImpl100) its current project's (ResearchLab_CurrentObjId of the game [0x7cf83c]'s lab) name: without a
// definition (ObjectDefs_Find in 0x7890a0) the language's text 0xb9 (Language_WideText), then ResearchMeters_Update (group); else the
// formatter's text 0x1be (0x7f characters, the table [0x78ba94]) of the full name (ObjectDefs_GetFullName, 0x42 characters; text argument
// 0xe) and the lab's Slots_GetValue of the group (a number, 0xd).
void __cdecl ResearchMeters_ShowName(u32 group);

// 0x4aaec0 (cdecl (group); name unconfirmed): the game [0x7cf83c]'s lab moved on to the group's next project (ResearchLab_NextProject) when
// its current one is not there or done (ResearchLab_CurrentNotDone); with the research panel [0x7cc1d8] up, ResearchMeters_ShowName and
// ResearchMeters_ShowProgress of the group.
void __cdecl ResearchMeters_Refresh(u32 group);

// 0x481520 (thiscall on the object at 0x7b51f0, this not used (group), pops 4): ResearchMeters_Refresh.
void __fastcall Obj7b51f0_RefreshResearch(void *, void *, u32 group);

// 0x4aa0c0 (cdecl (window, message, a, b); name invented): the handler set at 0x4aa740 (group 0's project arrows). On 0x100 the game
// [0x7cf83c]'s lab (GameTable7cfb90_Get1da722_519550) steps group 0's project back for the arrow 0xd2d03 (ResearchLab_PrevProject) or on
// for any other (ResearchLab_NextProject), then ResearchMeters_ShowName and ResearchMeters_ShowProgress of group 0. Every message then
// goes to LevelMeter_OnMessage, whose answer is returned.
s32 __cdecl ResearchMeters_ArrowProc(u8 *window, u32 message, u32 a, u32 b);
// 0x4aa360, 0x4aa3c0, 0x4aa420, 0x4aa300 (set at 0x4aa961, 0x4aaa72, 0x4aab82, 0x4aa850; names invented): ResearchMeters_ArrowProc for
// the groups 1, 2, 3 and 4.
s32 __cdecl ResearchMeters_ArrowProc1(u8 *window, u32 message, u32 a, u32 b);
s32 __cdecl ResearchMeters_ArrowProc2(u8 *window, u32 message, u32 a, u32 b);
s32 __cdecl ResearchMeters_ArrowProc3(u8 *window, u32 message, u32 a, u32 b);
s32 __cdecl ResearchMeters_ArrowProc4(u8 *window, u32 message, u32 a, u32 b);

// 0x4a96b0 (cdecl (window, message, a, b); name invented, unconfirmed): the research panel [0x7cc1d8]'s handler. 5: the panel sent 4 (0, 0), answer 1. 0x100 with a -2 or -1 the same, answer 0. The rest ends in
// Dialog_KeyHandler, whose answer is returned, after: 0x10 ResearchMeters_ShowProgress of 0, 4, 1, 2, 3. 0x14 the panel 0, Sound_Forward6 of the sounds [0x7cc1d0], [0x7cc1cc] and [0x7cc1d4] when set, Game_CancelAdvisor (0xc8).
// 0x15 the window's timer 0x80080 of 1000 set and, when that answers (AX) not 0, started; the hover label 0xd2d00 with the text 0xb8. 0x1e when Things_SumChain changed from [0x7cc1c8]: kept (the chain summed again), the limit [0x7cc1c0]
// (((100 - sum) / 3 + sum) << 10) / [0x7cc1b4], the slider 0xd2d10's child 0xd2d12 given it and 0xd2d10 given (the lab's master volume +0x1460 - 0x46) * limit / 30, each through slot 7 (all signed, truncated).
// 0x800 (a slider a moved to b): for the sliders 0xd2d0b-0xd2d0f Volumes_SetChannel 3, 2, 1, 0, 4 and, unless the byte [0x7cc1bc], ResearchMeters_Update (-1). 0xd2d10: b over the limit (signed) sets the slider back (slot 7) and is the limit; master volume
// b * 30 / limit + 0x46, the update as before, the five ShowProgress, 0xd2d10's child 0xd2d11 told (volume * 2 - 0x8c) / 7 (Window_Notify1), and Sound_SetParam 0xc then 0xd of [0x7cc1d0], [0x7cc1cc] to b * 100 / 1024. Any slider but 0xd2d10: Sound_SetParam
// [0x7cc1d4] 0xb then 0xa to b. 0x801, not 0xd2d10: [0x7cc1d4] the sound 0x9c (SoundUi_Play), its 0xb then 0xa to b. 0x802, not 0xd2d10: Sound_Stop of it, then 0.
s32 __cdecl ResearchPanel_Handler(u8 *window, u32 message, u32 a, u32 b);
}
