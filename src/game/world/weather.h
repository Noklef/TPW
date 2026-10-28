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

// 0x5131f0 (thiscall (mode), pops 4; name invented): on the weather renderer [0x8bcbac]: mode 0 its +0 = 1 (Obj_SetFirst) and, with no
// sound at +0x54, +0x54 = Sound_PlayAt(0, [0x803a20], 0x21, 0, 0, 0); mode 1 its +0 = 2; mode 2 +0x30 = 0 and no drops
// (WeatherFx_SetDropCount 0); modes 1 and 2 then stop the sound at +0x54 (Sound_Stop) and clear it. Other modes nothing.
void __fastcall Weather_SetMode(u8 *weather, void *, s32 mode);

// 0x512f00 (thiscall (value), pops 4; called by Weather_HandleMessage for message 1; name invented): +0xc = the value cut to 1..100;
// +0x10 = the funny time now (FunnyTime_Now on [0x7cf83c] + 0x2a0); +0x38 = whether it lies in the third range of [0x785504] (three
// [low, high] pairs, signed); in both the first and the second, a world random number's bit 0 picks the second (set) or the first.
// In the first: the share s = (value - low) * 100 / (high - low) (100 for an empty range; 32-bit, signed), Weather_SetMode 1,
// +0x34 = (100 - s) * [0x785530] / 100 at most [0x785530], the fall speed 0.8; in the second the same with its range, mode 0 and
// 2.5; in neither +0x34 = 0. Then +0x60 = (100 - value) * 0.01 cut to 0..1 and +0x5c = +0x5c - 45 + a world random number % 90,
// brought into 0..360 by steps of 360 (a NaN or infinity loops for ever, sic), in x87 single precision; the debug print of it
// being out of range is left out; the wind set (WeatherFx_SetWind: +0x5c, +0x60).
void __fastcall Weather_SetIntensity(u8 *weather, void *, s32 value);

// 0x5127a0 (thiscall (message), pops 4): the message's kind (its slot 0): 1, unless the byte +0x64 is set, Weather_SetIntensity with
// its +8; 2: +0x20 = the funny time now plus its +0xc days (FileTime_FromSpan, FileTime_Add; the span's out the message's own stack
// slot), +0x28 = its +8, +0x2c = 1, +0x18 = the funny time now. Other kinds nothing.
void __fastcall Weather_HandleMessage(u8 *weather, void *, u8 *message);

// 0x512000 (thiscall; name invented, unconfirmed): the weather taken out of the world (Thing_Detach), then the renderer's ([0x8bcbac])
// textures freed (WeatherFx_FreeTextures, a tail jump): returns its 1.
u32 __fastcall Weather_Detach(u8 *weather);
}
