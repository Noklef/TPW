// The language object (0x78b530: its fonts, "Unable to load font", and the name tables, e.g.
// THEMENAMES): reimplementation of 0x41a5a0 and 0x41a820-0x41a850 (thiscall), 0x485a70 and 0x4925a0. +4 picks which of
// four font sets (0x138 apart from +0x30) suits the screen; +8 is the Windows language id.
#pragma once

#include "../../../re/types.h"

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
}
