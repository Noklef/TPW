// A person's sprite script and ride timers: reimplementation of 0x4fdc90, 0x4fe950 and 0x50c0d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

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
