// The back surface shown: reimplementation of 0x564210.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// Calls by the game's address (the bench's recorders): DxResult_Message, EngineDebug_Log, EngineErrors_Push.
void Present_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x564210 (thiscall on the engine; name invented, unconfirmed): the hook [0x8bd510] first and [0x8bd514] last, when set. With the byte +0x780's bit 0 (full screen): flag bit 0 of [0x8bd508] set and +0x78c clear: a Blt
// (the front +0x10, slot 0x14) of the back +0x14: flag 0x8000000 set: cleared, and 0x1000000 until it answers 0; else once with the flag ~[0x8bcbc8]&1, its answers 0x8876021c and 0x887601ae taken as 0. Otherwise Flip
// (slot 0x2c; flag ~[0x8bcbc8]&1) while it answers those two, a budget of 50000000 lowered by 1 for the first and 50 for all others. A negative answer is logged (EngineDebug_Log(DxResult_Message)).
// Not full screen: flag 0x8000000 cleared; the Blt, 0x1000000, of all [0x8bcbd0] x [0x8bcbd4] to the window [0x8bd518]'s client area in screen terms (GetWindowRect and GetClientRect, 0x6fd390 and 0x6fd38c), while it
// answers those two; a failure goes to the engine's errors as kind 3. The answer is kept in [0x879cd4]; returns whether it is 0.
u32 __fastcall Device_Present(u8 *engine);
}
