// The language object (0x78b530: its fonts, "Unable to load font", and the name tables, e.g.
// THEMENAMES): reimplementation of 0x41a5a0 and 0x41a820-0x41a850 (thiscall). +4 picks which of
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
}
