// The dialog at 0x7ca308 (the window made from the template 0x751fa8 (id 0x247f) with the handler 0x49b650 by 0x49bf31; its child 0x2484
// shows the chosen staff member's figure, the script slot [0x7ca310]). Reimplementation of 0x49ba00. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x49ba00 (cdecl (index); called by 0x49b97b): the staff pool [0x7cf83c]'s entry index (StaffPool_ValidEntry) shown in the window
// [0x7ca308]'s child 0x2484 (Window_FindChild, Window_GetSavedRect), placed as Panel7cc328_ShowStaff places it (the floats [0x6ff984] /
// [0x6ff988], computed every time). Only for an entry whose type +0 / byte +9 differ from [0x7523e8] / [0x7523ec]: those kept, the
// script [0x7ca310] killed when set (SpriteScript_Kill), a new one started (SpriteScript_Start of 0x74f758, anim 1) into [0x7ca310]
// with the folder 5, 6, 4, 7, 8 for the types 0-4 (any other type: the index itself, sic) and the sprite the byte +9, its float
// register 8 set to 8.0 (SpriteScript_SetRegF). EAX not kept.
void __cdecl Dialog7ca308_ShowStaff(u32 index);
}
