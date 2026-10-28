// A GUI window that shows a level with an animated light (its child 0xd2d06; the callers about 0x4a9cc4 set four of them). Its state
// lives in the window's dwords +0xa0 (Window_DwordA0): 0 the light's position (16.16), 1 the clock when last moved, 2 the speed, 3 the
// phase (0-3). Reimplementation of 0x4a95b0, 0x4a9fc0, 0x4a9c70, 0x4a9bd0, 0x4aa120, 0x4aaec0 and 0x481520. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

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
}
