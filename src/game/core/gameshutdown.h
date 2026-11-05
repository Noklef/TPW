// The game's end: reimplementation of 0x5503f0, 0x550aa0, 0x40f230 and 0x44e560. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "addrcalls.h"

// Calls by the game's address (the bench's recorders): the modules' shut downs.
void GameShutdown_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x5503f0 (cdecl): with the game started ([0x879068]): at state 0xa ([0x87906c]) state 0xb and one more Game_RunFrame; then the owner, the front end's GUI, the key maps, movies, sound, TV static, the globals, scripts, flying
// rides and the advisor's speech let go in the original's order, and the temp folder removed (0x550aa0).
void __cdecl Game_Shutdown();

// 0x550aa0 (cdecl): a String of the memory source's path (0xf816c0's +0x28, as wide; vtable +0x18), ended in "\" with the name at 0x878eb8 appended (vtable +0x28), and that tree removed (GamePaths_RemoveTree).
void __cdecl Game_RemoveTempFolder();

// 0x40f230 (cdecl): GameOwner_ShutdownAll of the owner 0x786b68.
u32 __cdecl Game_ShutdownOwner();

// 0x44e560 (cdecl): Engine_ShutdownAll, Engine_DestroyDevice, Global879a24_Free; returns the last's answer.
u32 __cdecl Game_ShutdownEngine();
}
