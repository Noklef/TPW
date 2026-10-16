// A person's sprite script and ride timers: reimplementation of 0x4f9ed0, 0x4fdc90, 0x4fe900, 0x4fe950, 0x50be40 and 0x50c0d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4f9ed0 (thiscall): the script slot at +0xc is killed and cleared (Obj_KillSlot), the one at +0x30
// too (Person_KillScript84), and for a person of kind 1 (the byte +2) whose slot at +0x210 is set
// that one is jumped to its entry point (Person_JumpScript210).
void __fastcall Person_ReleaseScripts(u8 *person);

// 0x4fe900 (thiscall; a, b, c, d, pops 16): with the script at +0x210 set, SpriteScript_SetPosition on it
// with the position +0x218, b and +0x21c and r15 = d; then +0x218 = a and +0x21c = c.
// Nothing at all when the slot is 0.
void __fastcall Person_MoveScript(u8 *person, void *, u32 a, u32 b, u32 c, u32 d);

// 0x50be40 (thiscall): when the world's clock (the dword at +0x1da70c of the world that [0x7cf6ec] points
// through) is above +0x8c + 12 (unsigned) and the script slot at +0x84 is set, it is killed and cleared.
void __fastcall Person_KillScript84After(u8 *person);

// 0x4fdc90 (thiscall on a visitor): every 20th tick of the world's clock (the dword at +0x1da70c of
// [0x80239c], unsigned remainder 0) Visitor_PushPreviousRide with a ride whose low word is 0 and whose
// high word is the visitor's address's (sic: the argument is the address with its low word cleared).
void __fastcall Visitor_PreviousRideTick(u8 *visitor);

// 0x4fe950 (thiscall): when the script slot at +0x210 is set it is sent to the entry point 0x74f4c0
// (SpriteScript_Jump) and +0x210 cleared.
void __fastcall Person_JumpScript210(u8 *person);

// 0x50c0d0 (thiscall): when the script slot at +0x84 is set it is killed (SpriteScript_Kill) and +0x84
// cleared.
void __fastcall Person_KillScript84(u8 *person);
}
