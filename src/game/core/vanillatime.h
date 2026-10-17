// The game's module VANT (vanilla time): a dword at +0 that is a time relative to the game clock (Time_GetMs, the clock 0xfa71e0), saved as the clock's time
// now and loaded back as the saved time less the clock's time then: reimplementation of its save and load records 0x403220 / 0x403260 (names from re/symbols.csv).
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {

// 0x403220 (thiscall (stream), pops 4): writes the game clock's time now (Time_GetMs, 4 bytes; the object is not read); Stream_Write's result >= 0, always 1 (sic).
u32 __fastcall VanillaTime_Save(u8 *time, void *, Stream *stream);

// 0x403260 (thiscall (stream), pops 4): reads the 4 bytes saved into the stream argument's own stack slot (a short read leaves the pointer's bytes there, sic); when
// Stream_Read's result >= 0 (always) the dword at +0 becomes them less the clock's time now (Time_GetMs) and returns 1, else 0.
u32 __fastcall VanillaTime_Load(u8 *time, void *, Stream *stream);
}  // extern "C"
