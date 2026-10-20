// The global save info: the singleton at 0xf7c58c (made by 0x409410, 0x50 bytes, constructed by
// 0x5aef50) that keeps a record for each theme ("Global save info for a theme", 0x5b0a70) and
// the swear filter's word lists (0x5afd70). Reimplementation of its byte accessors 0x5af7f0-
// 0x5b0cc0 (thiscall), the sign names' setters (0x5b0940,
// 0x5b0970) and a theme record's save and load (0x5b0d70).
//
// Fields as 0x5aef50 sets them: +0x20 a counter (0), +0x24 a byte (0), +0x25 a byte (1), +0x26
// a byte array (its first u16 0), +0x48 the current theme's record, +0x4c a byte (1). A theme
// record's fields as 0x5b0d70 saves them: +0x34 mSignNameA, +0x76 mSignNameB, +0xb8 mNameChanged,
// +0xb9 mAllResearchCompleted.
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {

// 0x5b0d70 (thiscall on a theme's record; stream, save, an unused third; pops 0xc; name unconfirmed): the record written (save not 0;
// Stream_Write) or read (Stream_Read) field by field: the bytes +0-+5, then four times a byte (+6 on) and a dword (+0xc on), then 0x21
// times the u16s mSignNameA[i] (+0x34 on) and mSignNameB[i] (+0x76 on), then the bytes mNameChanged (+0xb8) and
// mAllResearchCompleted (+0xb9); the read takes those last ones through Save_ReadU16 / Save_ReadU8 (quiet). A field not all done
// (or a negative result): 1 at once. Else 0.
u32 __fastcall ThemeSaveRecord_Serialize(u8 *record, void *, Stream *stream, u32 save, u32);

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

// 0x5b0940 / 0x5b0970 (name; pops 4; names unconfirmed): with a current theme, its mSignNameA (+0x34) / mSignNameB (+0x76) given (the
// CRT's wcsncpy of 0x21 units: up to and with the NUL, the rest zeros; no NUL of its own when the name is as long) and its
// mNameChanged (+0xb8) set.
void __fastcall GlobalSave_SetSignNameA(u8 *info, void *, const u16 *name);
void __fastcall GlobalSave_SetSignNameB(u8 *info, void *, const u16 *name);

// 0x5b0ca0: the current theme's mAllResearchCompleted set, when there is a current theme.
void __fastcall GlobalSave_SetAllResearchCompleted(u8 *info);

// 0x5b0cb0 / 0x5b0cc0: byte +0x4c / cleared.
u8 __fastcall GlobalSave_Byte4c(const u8 *info);

// 0x5aef50 (thiscall): the global save info constructed: +4 a list's head (a 0x18-byte block from Mem_Alloc, unchecked, its two links
// itself) and +8 its size 0; +0x10 another's (Node18_New(0, 0)) and +0x14 0; +0x28 a tree of words (nil 0xf7c970 made the first time by
// Node14_New(0, 1), its users 0xf7c96c; the head Node14_New(nil, 0)) with the byte +0x30 0; +0x38 a tree of strings likewise (nil
// 0xf7c968 by Node28_New, users 0xf7c964) with +0x40 0; then +0x18, +0x1c, +0x20 0, the words +0x1c and +0x26 0, the bytes +0x24 0,
// +0x25 1 and +0x4c 1. The bytes +0, +0xc, +0x28, +0x29, +0x38 and +0x39 (the lists' and trees' allocators and the trees' comparators)
// get uninitialised stack bytes in the game (left as they are here); the STL lock around each tree is left out. Returns info.
u8 *__fastcall GlobalSave_Construct(u8 *info);

// 0x409410 (cdecl): the global save info (0xf7c58c), made the first time (Mem_Alloc(0x50), GlobalSave_Construct; none: it stays 0).
u8 *__cdecl GlobalSave_Instance();
void __fastcall GlobalSave_ClearByte4c(u8 *info);
}
