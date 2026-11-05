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

// 0x573890 (cdecl): Device_Present of the engine [0x879a24].
u32 __cdecl Present_Engine();

// 0x573cc0 (thiscall, a pointer inside the picture's object): the engine's back surface ([0x879a24]'s +0x14) locked (+0x64; its size 0x7c, flag 1) and the picture copied into it by the display mode ([0x8bcbc8], [0x8bcbe4]): with 0x40000000
// MmxBlit_Copy16 at depth 0x20, else MmxBlit_Dither16; without, MmxBlit_Copy32 at depth 0x20, else MmxBlit_Convert16, with the object's -0x10, the width doubled, 1 and the mask [0x8bd47c] when the mode has a bit of 0x4002000. Unlocked (+0x80), then
// Device_Present and its answer. A failed lock or unlock: its text in the engine's log, the engine's errors of kind 2 and 0. The answers are kept in [0x879cd4].
u32 __fastcall Present_Picture(u8 *picture);
}
