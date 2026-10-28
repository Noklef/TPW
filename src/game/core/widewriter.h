// A wide-text writer (vtable 0x70a0c0, set at 0x6acdb5 / 0x6ad0cd; the class name is invented): +4 the cursor (u16 cells), +8 the end,
// +0xc a table holder (its slot 0 releases it), +0x10 a byte set once the cursor reached the end. Its slot 4 is Obj_PutWord (methods.h).
// The code table (+0xc): +8 the first lead byte, +0xa the number of codes, +0xc the codes (u16). A byte below the lead byte is code
// byte - 1; a lead byte (and above) takes the next byte too: (byte + 0x100) * 0xff - lead * 0xfe + next, all 16-bit.
// Reimplementation of 0x6b00e0, 0x6aff70, 0x6afe80, 0x6affe0, 0x6ad0b0 and 0x6ad110; the names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b00e0 (slot 1, thiscall): with a cursor (+4, read once): the full byte (+0x10) set when it is 0 and the cursor is at the end
// (+8); then a 0 cell at the cursor while the byte is 0, else over the cell before it; the cursor cleared. Then a holder (+0xc) is
// cleared and released through its slot 0.
void __fastcall WideWriter_Finish(u8 *writer);

// 0x6aff70 (slot 2, thiscall (character)): with a code table, the character's low byte (and with a lead byte its high byte) decoded
// and its code (0 past the table's count) written through slot 4.
void __fastcall WideWriter_PutChar(u8 *writer, void *, u32 character);

// 0x6afe80 (slot 3, thiscall (text, count)): with a code table, the bytes decoded one character at a time (a lead byte taking the next
// one), each code (0 past the table's count) written through slot 4: count -1 up to the terminating 0 (none for an empty text), else
// until the text pointer reaches text + count (sic: compared for equality, so a lead byte last runs past it).
void __fastcall WideWriter_PutNarrow(u8 *writer, void *, const u8 *text, s32 count);

// 0x6affe0 (slot 5, thiscall (text, count)): the 16-bit cells copied to the cursor (+4, advanced) until the end (+8) sets the full
// byte (+0x10; nothing more written once it is set): count -1 up to the terminating 0 (none for an empty text), else count cells.
void __fastcall WideWriter_PutWide(u8 *writer, void *, const u16 *text, s32 count);

// 0x6ad0b0 (thiscall): the vtable set (0x70a0c0), finished (WideWriter_Finish, called directly), the range table (+0xc: left 0 by that) released
// through its slot 0 when there is one, then the base vtable (0x70a0a8).
void __fastcall WideWriter_Dtor(u8 *writer);

// 0x6ad110 (thiscall (flags), pops 4): WideWriter_Dtor, freed (GameHeap_Free) for flags bit 0; returns the writer.
u8 *__fastcall WideWriter_DeletingDtor(u8 *writer, void *, u32 flags);

}
