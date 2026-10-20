// The GUI library's window scripts: arrays of words read by 0x65fd58 (from 0x65fd0e) to build a window tree, the read position (in
// words, from the script's start) [0xfaa620]. Names unconfirmed. Reimplementation of 0x65fbf0, 0x65fc18, 0x65fc38 and 0x65fcc5.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x65fbf0 (cdecl (script); name unconfirmed): the dword at the read position; the position moves on two words.
u32 __cdecl WindowScript_ReadDword(u8 *script);

// 0x65fc18 (cdecl (script); name unconfirmed): the word at the read position in AX (EAX's upper half the old position's); the position
// moves on one word.
u32 __cdecl WindowScript_ReadWord(u8 *script);

// 0x65fc38 (cdecl (script, a, b, c, d); name unconfirmed): the next four words to *a, *b, *c and *d in turn (each read after the one
// before is stored); returns d.
u16 *__cdecl WindowScript_ReadRect(u8 *script, u16 *a, u16 *b, u16 *c, u16 *d);

// 0x65fcc5 (cdecl (script, a, b); name unconfirmed): the next two words to *a and *b in turn; returns b.
u16 *__cdecl WindowScript_ReadPair(u8 *script, u16 *a, u16 *b);
}
