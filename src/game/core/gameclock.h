// The game clock (GameClock_Now 0x4031e0 and its siblings, timed off timeGetTime): reimplementation
// of its four-byte save and load records 0x4030f0-0x4031b0 and the module's (CLOK) 0x402e00 /
// 0x402e30 (thiscall). +0x24 is the time the +0x20 clock is put back to (0x403160), +0x4c the one
// the +0x48 clock is (0x4031f0).
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

// 0x402e00 / 0x402e30, the module's save and load: +0x4c, then (when that worked) the +0x24 of
// the clock at +0x50.
u32 __fastcall Clock_Save(u8 *clock, void *, Stream *stream);
u32 __fastcall Clock_Load(u8 *clock, void *, Stream *stream);

// 0x403390 (thiscall): the double at +0x20 (1.0 from Obj_CtorOne), in st0. The game's movers scale their steps by it.
double __fastcall GameClock_Double20(const u8 *clock);

// The real-time clock (the one at +0x50 of the game's clock object 0x785970): +0 the last Time_GetMs, +8 the ms run (a double), +0x10 the time
// it was paused at, +0x14 the time spent paused, +0x18 paused, +0x20 the base its time is offset by, +0x24 the time it is put back to. All
// thiscall; names unconfirmed.
// 0x402f40: the ms since the last read added to +8 (x87, at the game's 24-bit precision); returns +8 __ftol'd (low half).
u32 __fastcall RealClock_Read(u8 *clock);
// 0x402f80 / 0x402fa0 / 0x402fd0: paused (the read time kept at +0x10) / resumed (the time since added to +0x14) / whichever it is not.
void __fastcall RealClock_Pause(u8 *clock);
void __fastcall RealClock_Resume(u8 *clock);
void __fastcall RealClock_TogglePause(u8 *clock);
// 0x403010: the read time (the time paused at, when paused) less +0x14. 0x403150: that plus +0x20. 0x403160: +0x20 set so that it is +0x24.
u32 __fastcall RealClock_Elapsed(u8 *clock);
u32 __fastcall RealClock_Now(u8 *clock);
void __fastcall RealClock_RestoreNow(u8 *clock);

// The game clock (the object at 0x785970 itself): +0x10 the last Time_GetMs, +0x18 the ms run (a double) at the rate +0x20 (a double), +0x28 the
// time it was paused at, +0x2c the time spent paused, +0x30 paused, +0x38 frozen (stepped by hand), +0x3c the frozen time, +0x40 its step,
// +0x44 the time spent frozen, +0x48 the base GameClock_Now is offset by, +0x4c the time it is put back to. All thiscall; names unconfirmed.
// 0x4033a0: the ms since the last read times +0x20 added to +0x18 (x87, 24-bit precision); returns +0x18 __ftol'd (low half).
u32 __fastcall GameClock_Read(u8 *clock);
// 0x403030 / 0x403050 / 0x403080: paused / resumed / whichever it is not (as the real-time clock's, at +0x28 - +0x30).
void __fastcall GameClock_Pause(u8 *clock);
void __fastcall GameClock_Resume(u8 *clock);
void __fastcall GameClock_TogglePause(u8 *clock);
// 0x4030d0: the read time (the time paused at, when paused) less +0x2c.
u32 __fastcall GameClock_Elapsed(u8 *clock);
// 0x402ea0 (frames a second, pops 4): unless frozen, frozen at the elapsed time plus +0x44 with the step 1000 / frames (unsigned; 0 faults, sic).
void __fastcall GameClock_Freeze(u8 *clock, void *, u32 fps);
// 0x402ed0: when frozen, thawed, +0x44 set so that the time goes on from the frozen time.
void __fastcall GameClock_Unfreeze(u8 *clock);
// 0x402f10: the frozen time, or the elapsed time plus +0x44. 0x4031e0: that plus +0x48. 0x4031f0: +0x48 set so that it is +0x4c.
u32 __fastcall GameClock_Time(u8 *clock);
u32 __fastcall GameClock_Now(u8 *clock);
void __fastcall GameClock_RestoreNow(u8 *clock);

// The pair (thiscall on the game clock; the real-time clock at +0x50; names unconfirmed): 0x402d70 GameClock_Now; 0x402d80 the real-time clock's
// RealClock_Now; 0x402d90 / 0x402db0 / 0x402dd0 both paused / resumed / toggled (the game clock first); 0x402e60 both times kept to be put back to
// (+0x4c, +0x74); 0x402e80 both put back.
u32 __fastcall Clocks_Now(u8 *clock);
u32 __fastcall Clocks_RealNow(u8 *clock);
void __fastcall Clocks_Pause(u8 *clock);
void __fastcall Clocks_Resume(u8 *clock);
void __fastcall Clocks_TogglePause(u8 *clock);
void __fastcall Clocks_SaveNow(u8 *clock);
void __fastcall Clocks_RestoreNow(u8 *clock);

// 0x4033e0 / 0x403400 (cdecl; names unconfirmed): static initialisers of two floats nothing reads: 0x785a04 = 1 and 0x785ddc = 1 / 2^23 (as
// camera's).
void __cdecl GameClock_InitUnusedOne();
void __cdecl GameClock_InitUnusedEpsilon();
}
