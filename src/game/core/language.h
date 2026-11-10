// The language object (0x78b530: its fonts, "Unable to load font", and the name tables, e.g.
// THEMENAMES): reimplementation of 0x41a5a0 and 0x41a820-0x41a850 (thiscall), 0x485a70 and 0x4925a0. +4 picks which of
// four font sets (0x138 apart from +0x30) suits the screen; +8 is the Windows language id.
#pragma once

#include "../../../re/types.h"
#include "addrcalls.h"

extern "C" {

// 0x41a5a0: +4 = the font set for a screen width: 0 up to 512, 1 up to 640, 2 up to 800, else 3.
void __fastcall Language_SetFontSet(u8 *lang, void *, s32 width);

// 0x41a820: the buffer at 0xf7f230, which the callers take as a string.
const char *__fastcall Language_Buffer(const u8 *lang);

// 0x41a830 / 0x41a840: the language id (+8).
u32 __fastcall Language_Id(const u8 *lang);
void __fastcall Language_SetId(u8 *lang, void *, u32 id);

// 0x41a850: whether it is 0x40d, Hebrew.
u32 __fastcall Language_IsHebrew(const u8 *lang);

// 0x485a70 (cdecl): font index of the current set (+4), 0x18-byte entries 13 to a set from
// +0x1c; null from 13 on (unsigned).
u8 *__cdecl Language_Font(u32 index);

// 0x485ac0 (cdecl (text); name unconfirmed): 0 for no text; otherwise the static wide buffer 0x7ba058, filled from the text first
// (Wide_FromNarrow, core/methods, by the converter [0x78ba7c], at most 0x200 characters; its status into the argument's own slot) only
// while [0x78ba80] is set.
u16 *__cdecl Language_Widen(const u8 *text);

// 0x485b00 (cdecl (index); name unconfirmed): the text the record table [0x78ba94] holds at the index (Records_DataIfTerminated,
// core/methods), widened as Language_Widen does; 0 when there is none.
u16 *__cdecl Language_WideText(u32 index);

// 0x485f00 (cdecl (object, index); name unconfirmed): the text of the record table [0x78baa4] at the index, widened as Language_Widen
// does (0 when there is none), handed to the object's slot 9 (thiscall, one argument); returns what that returns.
u32 __cdecl Language_SetObjectText(u8 *object, u32 index);

// 0x4925a0 (cdecl): in Hebrew, an alignment of 0 becomes 2 and 2 becomes 0.
void __cdecl Language_MirrorAlign(u32 *align);

// 0x41a5f0 (thiscall; name invented, unconfirmed): +0 = 0; the fonts of the four sets (+0x30, 0x18 apart, 13 to a set, sets 0x138
// apart; by entry, then set) and the objects at +0x54c to +0x5a4 (+0x58c after +0x594) each released through its slot 0 (thiscall, no
// arguments) and cleared when set; then the resource folder at +0xc closed (ResFolder_Close; its result a local).
void __fastcall Language_FreeFonts(u8 *lang);

// 0x419570 (thiscall; the object at 0x78b530's static constructor's; name invented, unconfirmed): +0 = 0, +4 = 1; the resource folder
// (+0xc, ResFolder_Ctor), the 0x34 font entries (+0x1c, 0x18 bytes: 0x419680, Obj_CtorValue with 0; CRT_VectorCtorIterator) and
// the text style (+0x4fc, TextStyle_Ctor) constructed; +0x54c to +0x59c cleared but +0x560 and +0x590; each set's entry (+0x2c + set *
// 0x138 + entry * 0x18) its index, its font (+4) none. Returns the object.
u8 *__fastcall Language_Ctor(u8 *lang);

// 0x4196a0 (thiscall; name invented, unconfirmed): the text style (+0x4fc), the font entries (0x47f980: their blocks +0xc freed; the last first)
// and the resource folder (+0xc) destroyed.
void __fastcall Language_Dtor(u8 *lang);

// 0x419710 (thiscall; name unconfirmed): the language's resources from its folder (+0xc), each callee's answer a pointer to a status that
// fails below 0. The folder "Data\\Language\\" opened (0x6ad1b0) and the file at 0xf7f230 loaded (0x6ad230): a failure is
// GameError_Set(0x785de8, 3, 0, the name) and 0. The range table (0x6ad4b0: +0x550, key 2) and code table (0x6ad460: +0x54c, key 1): a
// failure is -1. 21 charsets (0x6ad510: +0x554 on, 4 apart): a failure formats its two names (sprintf, 0x74c67c) for OutputDebugStringA,
// that error with the name, 0. One font set of 13 (0x6ad570: 0x18 apart, 0x138 from set to set, from +0x30), set by [0x7858ec] when above 0
// (1 to 4 and up: sets 0 to 3), else by [0x78d8dc] (0, 1, 2, other: sets 0 to 3): a failure is that error with 0x74c390, 0. Then +0x4fc's
// status (0x6ad790), +0 = 1, 1.
s32 __fastcall Language_Load(u8 *lang);

// 0x419680 (thiscall; name invented, unconfirmed): Obj_CtorValue with 0 and its answer.
u32 __fastcall Language_CtorValue0(u8 *self);
}

// Test seam: fncmp's driver sends Language_Load's calls to recorders.
void Language_SetCalls(const AddrCall *table, u32 count);

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const char[0x10], gLanguage_6fd990, 0x006fd990, 0x10);
GAME_DATA(char[0x8], gLanguage_74c390, 0x0074c390, 0x8);
GAME_DATA(char[0xc], gLanguage_74c398, 0x0074c398, 0xc);
GAME_DATA(char[0x14], gLanguage_74c3a4, 0x0074c3a4, 0x14);
GAME_DATA(char[0x14], gLanguage_74c3b8, 0x0074c3b8, 0x14);
GAME_DATA(char[0x14], gLanguage_74c3cc, 0x0074c3cc, 0x14);
GAME_DATA(char[0x10], gLanguage_74c3e0, 0x0074c3e0, 0x10);
GAME_DATA(char[0x18], gLanguage_74c3f0, 0x0074c3f0, 0x18);
GAME_DATA(char[0x10], gLanguage_74c408, 0x0074c408, 0x10);
GAME_DATA(char[0x18], gLanguage_74c418, 0x0074c418, 0x18);
GAME_DATA(char[0x10], gLanguage_74c430, 0x0074c430, 0x10);
GAME_DATA(char[0x14], gLanguage_74c440, 0x0074c440, 0x14);
GAME_DATA(char[0xc], gLanguage_74c454, 0x0074c454, 0xc);
GAME_DATA(char[0x18], gLanguage_74c460, 0x0074c460, 0x18);
GAME_DATA(char[0x10], gLanguage_74c478, 0x0074c478, 0x10);
GAME_DATA(char[0x18], gLanguage_74c488, 0x0074c488, 0x18);
GAME_DATA(char[0x14], gLanguage_74c4a0, 0x0074c4a0, 0x14);
GAME_DATA(char[0x1c], gLanguage_74c4b4, 0x0074c4b4, 0x1c);
GAME_DATA(char[0xc], gLanguage_74c4d0, 0x0074c4d0, 0xc);
GAME_DATA(char[0x18], gLanguage_74c4dc, 0x0074c4dc, 0x18);
GAME_DATA(char[0x14], gLanguage_74c4f4, 0x0074c4f4, 0x14);
GAME_DATA(char[0x1c], gLanguage_74c508, 0x0074c508, 0x1c);
GAME_DATA(char[0x10], gLanguage_74c524, 0x0074c524, 0x10);
GAME_DATA(char[0x18], gLanguage_74c534, 0x0074c534, 0x18);
GAME_DATA(char[0x10], gLanguage_74c54c, 0x0074c54c, 0x10);
GAME_DATA(char[0x18], gLanguage_74c55c, 0x0074c55c, 0x18);
GAME_DATA(char[0x10], gLanguage_74c574, 0x0074c574, 0x10);
GAME_DATA(char[0x18], gLanguage_74c584, 0x0074c584, 0x18);
GAME_DATA(char[0x10], gLanguage_74c59c, 0x0074c59c, 0x10);
GAME_DATA(char[0x14], gLanguage_74c5ac, 0x0074c5ac, 0x14);
GAME_DATA(char[0xc], gLanguage_74c5c0, 0x0074c5c0, 0xc);
GAME_DATA(char[0x14], gLanguage_74c5cc, 0x0074c5cc, 0x14);
GAME_DATA(char[0xc], gLanguage_74c5e0, 0x0074c5e0, 0xc);
GAME_DATA(char[0x14], gLanguage_74c5ec, 0x0074c5ec, 0x14);
GAME_DATA(char[0x8], gLanguage_74c600, 0x0074c600, 0x8);
GAME_DATA(char[0x10], gLanguage_74c608, 0x0074c608, 0x10);
GAME_DATA(char[0xc], gLanguage_74c618, 0x0074c618, 0xc);
GAME_DATA(char[0x14], gLanguage_74c624, 0x0074c624, 0x14);
GAME_DATA(char[0x10], gLanguage_74c638, 0x0074c638, 0x10);
GAME_DATA(char[0x14], gLanguage_74c648, 0x0074c648, 0x14);
GAME_DATA(char[0xc], gLanguage_74c65c, 0x0074c65c, 0xc);
GAME_DATA(char[0x14], gLanguage_74c668, 0x0074c668, 0x14);
GAME_DATA(char[0x28], gLanguage_74c67c, 0x0074c67c, 0x28);
GAME_DATA(char[0x10], gLanguage_74c6a4, 0x0074c6a4, 0x10);
GAME_DATA(char[0x14], gLanguage_74c6b4, 0x0074c6b4, 0x14);
GAME_DATA(char[0x108], gLanguage_f7f230, 0x00f7f230, 0x108);
