// A game-time stamp (TimeSig): one dword, the game's time when it was taken (the accessors 0x41a960-0x41a980 set and age it). Reimplementation
// of its save game serializer 0x41a860.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x41a860 (thiscall (stream, save, version, unused)): a version other than 0 or 1 returns 0. Otherwise the field is announced
// (Save_FieldName(4, name, !save)) and the stamp's 4 bytes written to (save) or read from the stream; 0 when the transfer reports no error and
// all 4 bytes went, else 1 (after an assert, left out).
u32 __fastcall TimeSig_Serialize(u8 *self, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
