// The GUI library's window scripts: arrays of words read by 0x65fd58 (from 0x65fd0e) to build a window tree, the read position (in
// words, from the script's start) [0xfaa620]. Names unconfirmed. Reimplementation of 0x65fbf0, 0x65fc18, 0x65fc38, 0x65fcc5, 0x65fd58 and 0x65fd0e.
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

// 0x65fd58 (thiscall (script), pops 4; name unconfirmed): the window's script run from the read position: opcodes (words) until 5 or one
// past 0x12. 0: a child (Window_CreateChild: kind word, flags and id dwords, then a rectangle) and its own script; 1 / 2: a dword hash
// found (GuiWindows_FindByHash) made the window's +0x98 object / +0x9c impl; 3: a rectangle to +0x2c (Window_OffsetByParents of it with
// flags 0x400); 4: a hit area (1 a rectangle, 2 a point and a radius, 3 an ellipse, 4 a count and its points, each built in a 0xc-byte
// block; another sub-opcode nothing); 6 / 0xe, 7 / 0xf: a button child (kind 2, flags 0x21, id 1 / 2); 8: a button (2, 1, id 3) with
// the handler 0x66bb9b; 9: for kind 4 or 7 (slot 3) with no child of id 1, a scroll bar child (3, 0x10, 1); 0xa: for kind 4 / 7 a
// rectangle as the inner one; 0xb: a column count and its columns (kind 7's); 0xc: a button (2, 1, id word + 0x10); 0xd: kind 11's range
// (two words, sign-extended); 0x10: a kind 6 child (flags 1, id word | 0x11100000); 0x11 / 0x12: a dword to +0x90 / +0x94. The
// children's scripts follow in the same stream. Returns the last child made here (0 for none).
u8 *__fastcall WindowScript_Run(u8 *window, void *, u8 *script);

// 0x65fd0e (thiscall (script, handler), pops 8; name unconfirmed): the read position [0xfaa620] reset and the script run on the window
// (WindowScript_Run); a handler not 0 becomes the last child's when there is one (Window_SetHandler). Returns that child.
u8 *__fastcall WindowScript_Build(u8 *window, void *, u8 *script, u32 handler);
}
