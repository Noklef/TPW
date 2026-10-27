// An object's reference to a running sprite script: reimplementation of 0x4d4100-0x4d4190 and 0x4d3ee0.
//
// The object keeps the script's slot at +0 (0 for none), the animation it was last put at (a number
// into the table of script entry points at 0x75a418) at +4 and a period to set at +8.
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {

// 0x4d3ee0 (thiscall; stream, save, version, unused): the dwords +0, +4 and +8 (mSpriteScript, mNextAnim, mNextServiceInterval) saved or
// loaded as a serializer does (Save_Field).
u32 __fastcall SpriteAnim_Serialize(u8 *ref, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4d4140 (thiscall; anim, folder, sprite, pops 12): starts a script at entry point anim (the table's)
// with the folder and sprite (SpriteScript_Start, no position); +0 = its slot.
void __fastcall ScriptRef_Start(u8 *ref, void *, u32 anim, u32 folder, u32 sprite);

// 0x4d4100 (thiscall; anim, pops 4): 1 when the script is at entry point anim (SpriteScript_IsAt) or,
// when it is not, when anim is the one in +4; else 0.
u32 __fastcall ScriptRef_IsAt(const u8 *ref, void *, u32 anim);

// 0x4d4190 (thiscall): when +4 is set the script is sent to that entry point (SpriteScript_Jump) and
// +4 cleared; when +8 is set it is made the script's period (SpriteScript_SetPeriod) and +8 cleared.
void __fastcall ScriptRef_Apply(u8 *ref);

// 0x4d41d0 (thiscall (speed, a float), pops 4; called by 0x4fa442 on a person's +0xc; name invented): the period +8 set from the
// speed: __ftol(62 / speed) (the x87's quotient at the game's precision) clamped to 0-250 (an invalid quotient's __ftol 0).
void __fastcall ScriptRef_SetPeriodFor(u8 *ref, void *, float speed);

// 0x4d4170 (thiscall): the script at +0 killed (SpriteScript_Kill) and +0 = 0.
void __fastcall Obj_KillSlot(u8 *object);
}
