// The game's owner object (the registry's owner, the global 0x786b68): its pausing and its end.
#pragma once

#include "addrcalls.h"
#include "../../../re/types.h"

// Its calls go by the game's addresses; a table of replacements (tests/fncmp/gameowner.py gives them recorders).
void GameOwner_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x4092a0 (thiscall; mode, flag; pops 8; names here unconfirmed): with +0x3c 1: +0x1c set; mode 1 opens the front end on the GUI (0x7b51f0); the clocks paused; the flag's byte kept at +0x20:
// set, Sound_Post700b64; clear, the advisor's speech held and Sound_Set803ad2 (1).
void __fastcall GameOwner_Pause(u8 *self, void *, s32 mode, u32 flag);
// 0x409300 (thiscall): with +0x3c 1: +0x1c cleared, the front end told, the clocks resumed; by +0x20 Sound_Post700b68, or the speech let go and Sound_Set803ad2 (0).
void __fastcall GameOwner_Resume(u8 *self);
// 0x409350 (thiscall; flag; pops 4): with +0x3c 1: the front end told, the clocks toggled; paused (+0x1c 1) as 0x409300's end, else as 0x4092a0's.
void __fastcall GameOwner_TogglePause(u8 *self, void *, u32 flag);
// 0x409180 (thiscall; returns 1): the level at [0x7cf83c] cleared, destroyed and freed; the object definitions and the landscape's textures let go; with flag 0x400000 of 0x8bcbc8 the
// texture list at +0x38 emptied and freed; the message centre at [0x788d3c] destroyed and freed; the named globals removed.
u32 __fastcall GameOwner_Shutdown(u8 *self);
// 0x409150 (thiscall; name invented, unconfirmed): GameOwner_Shutdown; the mouse mode at [0x7b05e8], if any, set to 0 (MouseMode_Set) and freed; the global cleared.
void __fastcall GameOwner_ShutdownAll(u8 *self);
}
