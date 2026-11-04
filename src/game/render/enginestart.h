// The engine's start: DirectDraw opened, the Direct3D device for the driver kind made and the engine's internals created, with a fall back from kind 1 to kind 4. Reimplementation of
// 0x44df20, 0x5632b0, 0x572780, 0x5836d0 and 0x583700.
//
// The driver kind is [0x8bd524]: 1 and 4 have a device (0x572c40 and 0x573960, not ported yet), others none.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// The callees, by the game's addresses (the driver's: recorders). nullptr restores the game's.
void EngineStart_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x44df20 (cdecl; name invented): DirectDraw opened ([0x7a130c], [0x7a1968], [0x7a1930], then the arguments a, b, c), the device (1) made and the engine's internals created (d, 0x92f2266, 0). Unless the
// kind is 4 already, a failed open, a kind 1 device or a kind 1 engine goes on with kind 4; what was made is taken down on the way out. 1 when all were made, else 0.
u32 __cdecl Engine_Start(u32 a, u32 b, u32 c, u32 d);

// 0x5632b0 (cdecl; name invented): 0 with the object at [0x879a24] already. A new one (0x790 bytes; none: the error (3, 0xb0754000) pushed, 0) opened (DirectDraw_Open; x1..x5, whether the kind is 1), x0 in
// [0x8bd518] first. Opened, 1. Else it is released (Obj_ReleaseSevenComMembers) and freed, [0x879a24] cleared, the error (3, 0xb0754004) pushed, 0.
u32 __cdecl Engine_OpenDirectDraw(u32 x0, u32 x1, u32 x2, u32 x3, u32 x4, u32 x5);

// 0x572780 (cdecl; second argument not used): the engine's parts created one after the other, by the driver kind, the first that answers 0 ending it: its parts all taken down (in a fixed order, those never made too)
// and 0. Else the wind set and the settings fitted, 1. With [0x8bd508] & 0x2000000 each stage logged (EngineDebug_LogFallback), by sprintf into a buffer but for the first and last texts. a is the texture cache's argument, c the
// procedural texture's file (0: none).
u32 __cdecl Engine_Init(u32 a, u32 unused, u32 c);

// 0x5836d0 (cdecl; name invented): the device made for the kind (1: 0x572c40, 4: 0x573960), its answer; other kinds 0.
u32 __cdecl Engine_CreateDevice(u32 x);

// 0x583700 (cdecl; name invented): the device taken down for the kind. Kind 1: D3dSetup_Teardown, its answer. Kind 4: 0x573b30, 0. Other kinds 0.
u32 __cdecl Engine_DestroyDevice();
}
