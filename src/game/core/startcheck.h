// The startup's check that the game may run: installed (Registry_GameInstalled) and not already running (a named mutex).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x45f640 (cdecl u8; name unconfirmed): 0 with game error 0 (GameError_Set on the report 0x785de8: index 0, no argument or text) when
// the game is not installed (Registry_GameInstalled). Otherwise CreateMutexA (no security, owned, the name at [0x7a2508]; import slot
// 0x6fd134) kept at 0x7a267c: when one is made and GetLastError (0x6fd138) is ERROR_ALREADY_EXISTS (0xb7), game error 1 and the mutex
// closed (CloseHandle, 0x6fd13c; the handle stays at 0x7a267c), 0. Else 1 (a failed CreateMutexA too).
u8 __cdecl Startup_CheckCanRun();
}
