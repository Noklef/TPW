// The game clock (GameClock_Now 0x4031e0 and its siblings, timed off timeGetTime): reimplementation
// of its four-byte save and load records 0x4030f0-0x4031b0 (thiscall). +0x24 is the time the +0x20
// clock is put back to (0x403160), +0x4c the one the +0x48 clock is (0x4031f0).
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {

// 0x4030f0 / 0x403180: writes +0x24 / +0x4c to the stream. 0x403120 / 0x4031b0: reads it back.
// Each gives Stream_Write's or Stream_Read's result >= 0, which always holds (sic: 1 either way).
u32 __fastcall GameClock_Save24(u8 *clock, void *, Stream *stream);
u32 __fastcall GameClock_Load24(u8 *clock, void *, Stream *stream);
u32 __fastcall GameClock_Save4c(u8 *clock, void *, Stream *stream);
u32 __fastcall GameClock_Load4c(u8 *clock, void *, Stream *stream);
}
