// UTF-16 text scanning for the object definition parser (0x413600-0x41389e: a BOM-skipping
// keyword match against the table 0x7487f8, then a line at a time): reimplementation of
// 0x413380 and 0x4138b0. Separators are 0, space, tab, LF and CR; line ends LF, CR, CR LF or
// LF CR.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x413380: 1 when a and b hold the same word (the same characters up to a separator on both
// sides; the separators themselves may differ), else 0.
u32 __cdecl WText_WordEquals(const u16 *a, const u16 *b);

// 0x4138b0: the start of the first line after p's that is not blank (spaces and tabs only),
// leading blanks included; the terminator when there is none.
const u16 *__cdecl WText_NextLine(const u16 *p);
}
