// The weather renderer made and its random tables filled: reimplementation of 0x57d100 and 0x57d300. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// Calls by the game's address (the bench's recorders): Render3d_Init and EngineErrors_Push.
void WeatherTables_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x57d100 (cdecl): with no renderer ([0x8bcbac] 0) a block of 0xa1cc built by Render3d_Init (no block: 0), stored; none: EngineErrors_Push(3, 0xb0754000) on 0x879ad0 and 0; else its tables filled and 1. One already there: 0.
u32 __cdecl WeatherFx_Create();

// 0x57d300 (thiscall): from CRT rand and the constants at 0x701c10-0x701c94: +8, 0x400 points in a cube; +0x3358, 0x80 of a cosine's table; +0x3558, +0x4558, +0x5558, 0x400 each of the drops' start, speed and direction; from 0x80 random
// points on the stack, in a ring, +0x8558, 0x80 records of 0x30 bytes (a cubic's differences g, h, k and its point, x87 at 24 bits); +0x3344 = timeGetTime. Returns 1.
u32 __fastcall WeatherFx_FillTables(u8 *fx);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const float, gWeathertables_701c6c, 0x00701c6c, 0x4);
GAME_DATA(const float, gWeathertables_701c70, 0x00701c70, 0x4);
GAME_DATA(const float, gWeatherFx_FillTables_701c74, 0x00701c74, 0x4);
GAME_DATA(const double, gWeatherFx_FillTables_701c78, 0x00701c78, 0x8);
GAME_DATA(const float, gWeatherFx_FillTables_701c80, 0x00701c80, 0x4);
GAME_DATA(const float, gWeatherFx_FillTables_701c88, 0x00701c88, 0x4);
GAME_DATA(const float, gWeatherFx_FillTables_701c8c, 0x00701c8c, 0x4);
GAME_DATA(const float, gWeathertables_701c90, 0x00701c90, 0x4);
GAME_DATA(const float, gWeatherFx_FillTables_701c94, 0x00701c94, 0x4);
