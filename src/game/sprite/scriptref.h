// An object's reference to a running sprite script: reimplementation of 0x4d4100-0x4d4190.
//
// The object keeps the script's slot at +0 (0 for none), the animation it was last put at (a number
// into the table of script entry points at 0x75a418) at +4 and a period to set at +8.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d4140 (thiscall; anim, folder, sprite, pops 12): starts a script at entry point anim (the table's)
// with the folder and sprite (SpriteScript_Start, no position); +0 = its slot.
void __fastcall ScriptRef_Start(u8 *ref, void *, u32 anim, u32 folder, u32 sprite);

// 0x4d4100 (thiscall; anim, pops 4): 1 when the script is at entry point anim (SpriteScript_IsAt) or,
// when it is not, when anim is the one in +4; else 0.
u32 __fastcall ScriptRef_IsAt(const u8 *ref, void *, u32 anim);

// 0x4d4190 (thiscall): when +4 is set the script is sent to that entry point (SpriteScript_Jump) and
// +4 cleared; when +8 is set it is made the script's period (SpriteScript_SetPeriod) and +8 cleared.
void __fastcall ScriptRef_Apply(u8 *ref);
}
