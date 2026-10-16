// The global save info: the singleton at 0xf7c58c (made by 0x409410, 0x50 bytes, constructed by
// 0x5aef50) that keeps a record for each theme ("Global save info for a theme", 0x5b0a70) and
// the swear filter's word lists (0x5afd70). Reimplementation of its byte accessors 0x5af7f0-
// 0x5b0cc0 (thiscall).
//
// Fields as 0x5aef50 sets them: +0x20 a counter (0), +0x24 a byte (0), +0x25 a byte (1), +0x26
// a byte array (its first u16 0), +0x48 the current theme's record, +0x4c a byte (1). A theme
// record's fields as 0x5b0d70 saves them: +6 mSignNameA, +0x76 mSignNameB, +0xb8 mNameChanged,
// +0xb9 mAllResearchCompleted.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5af7f0 (i): byte i of the current theme's record (in AL only, as all the byte getters).
u8 __fastcall GlobalSave_ThemeByte(const u8 *info, void *, u32 i);

// 0x5af800 (i): byte +0x26 + i.
u8 __fastcall GlobalSave_Byte26(const u8 *info, void *, u32 i);

// 0x5afc30: the counter +0x20 incremented.
void __fastcall GlobalSave_Inc20(u8 *info);

// 0x5afc40 / 0x5afc50 (value): byte +0x24 / set. 0x5b09b0 / 0x5b09a0 (value): byte +0x25 / set.
u8 __fastcall GlobalSave_Byte24(const u8 *info);
void __fastcall GlobalSave_SetByte24(u8 *info, void *, u32 value);
u8 __fastcall GlobalSave_Byte25(const u8 *info);
void __fastcall GlobalSave_SetByte25(u8 *info, void *, u32 value);

// 0x5b0ca0: the current theme's mAllResearchCompleted set, when there is a current theme.
void __fastcall GlobalSave_SetAllResearchCompleted(u8 *info);

// 0x5b0cb0 / 0x5b0cc0: byte +0x4c / cleared.
u8 __fastcall GlobalSave_Byte4c(const u8 *info);
void __fastcall GlobalSave_ClearByte4c(u8 *info);
}
