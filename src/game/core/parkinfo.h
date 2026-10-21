// The save file's park information (read by the header code, 0x415f50 / 0x416240): u16 strings at +0 (0x10) and +0x20 (0x80), three
// dwords +0x120..+0x128, u16 strings at +0x12c (0x10), the author's name +0x14c (0x10), e-mail +0x16c (0x32), the park's description
// +0x1d0 (0x32) and the publishing date's day, month and year (u16 +0x3ce, +0x3d0, +0x3d2). Reimplementation of its serializer 0x418da0.
// The names are unconfirmed (the fields' from the strings the loader announces them by).
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x418da0 (thiscall (stream, save, version, unused), pops 0x10; name unconfirmed): a version other than 0 or 1 returns 0. Saving, every
// field goes out in order through Stream_Write, each u16 of the strings by itself; loading, the fields up to +0x14c come in through
// Stream_Read the same way and the rest through Save_ReadU16 (quiet, by the names "strAuthorPlayerName[%i]" ... "wParkDatePublished_y").
// The first transfer that reports an error or moves less returns 1 (after an assert, left out), as does a failed Save_ReadU16; else 0.
u32 __fastcall ParkInfo_Serialize(u8 *self, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
