// The weather (thing model 15): reimplementation of its serializer 0x512010. After the thing, it holds the dword at +0xc, the 8-byte fields
// at +0x10 and +0x18, a forecast record (world/forecast) at +0x20, then the fields from +0x30 to +0x64.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x512010 (thiscall; stream, save, version, unused): the thing (Thing_Serialize; 1 if it fails), then saved or loaded as a serializer does
// (Save_Field; the load path partly through the readers): +0xc (4), +0x10 (8), +0x18 (8), the forecast (Forecast_Serialize; 1 if it
// fails), +0x30, +0x34 (4), +0x38 (1), +0x3c, +0x40, +0x44 (4), +0x48 (12), +0x54, +0x58, +0x5c, +0x60 (4), +0x64 (1).
u32 __fastcall Weather_Serialize(u8 *weather, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
