// The weather (thing model 15): reimplementation of its serializer 0x512010 and its lightning bolts' ends 0x512c50. After the thing, it holds
// the dword at +0xc, the 8-byte fields at +0x10 and +0x18, a forecast record (world/forecast) at +0x20, then the fields from +0x30 to +0x64.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x512010 (thiscall; stream, save, version, unused): the thing (Thing_Serialize; 1 if it fails), then saved or loaded as a serializer does
// (Save_Field; the load path partly through the readers): +0xc (4), +0x10 (8), +0x18 (8), the forecast (Forecast_Serialize; 1 if it
// fails), +0x30, +0x34 (4), +0x38 (1), +0x3c, +0x40, +0x44 (4), +0x48 (12), +0x54, +0x58, +0x5c, +0x60 (4), +0x64 (1).
u32 __fastcall Weather_Serialize(u8 *weather, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x512c50 (thiscall (&start, &end, unused), pops 12, the weather unused; name unconfirmed): a lightning bolt's random ends (its caller
// hands them on with the weather's +0x44). With r() a world random number's low 30 bits times 2^-30 (World_Rand on [0x7cf83c]) and
// the map's size in units (the cells [0x7a0314] / [0x7a0374] times 10): start = (r() * width, 0, r() * height), end = (r() * 100 +
// start x - 50, 300, r() * 100 + start z - 50), in that order, in x87 single precision; then start's x and z cut to the size and
// raised to 1 when below it.
void __fastcall Weather_RandomBolt(u8 *weather, void *, float *start, float *end, u32 unused);
}
