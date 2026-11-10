// A forecast record (its constructor is initializers' 0x513290): reimplementation of its serializer 0x5132a0. It holds mTime (8 bytes at +0),
// mQuality (the dword at +8) and mValid (the byte at +0xc).
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {
// 0x5132a0 (thiscall; stream, save, version, unused): mTime, mQuality and mValid saved or loaded as a serializer does (Save_Field).
u32 __fastcall Forecast_Serialize(u8 *forecast, void *, Stream *stream, u32 save, s32 version, u32 unused);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[8], gText_Forecast_Serialize_762418, 0x00762418, 0x8);
