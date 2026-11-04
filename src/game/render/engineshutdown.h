// The engine's shutdown.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

void EngineShutdown_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x572b60 (name invented, unconfirmed): the engine's objects destroyed in order, render mode 1 (0x8bd524) and mode 4 with their own. Returns 1.
u32 __cdecl Engine_ShutdownAll();

}
