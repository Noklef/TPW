// A narrow-text writer (vtable 0x70a090; the class name is invented), the wide-text writer's (widewriter.h) sibling over bytes: +4 the
// cursor, +8 the end, +0xc the range table holder (Ranges_Lookup's object; its slot 0 releases it), +0x10 a byte set once the cursor
// reached the end. Its slot 3 is ByteBuf_Write (seqs.h); slot 2 writes a character's bytes and slots 4 and 5 narrow 16-bit characters
// through the range table into it.
// Reimplementation of 0x6afe40, 0x6afd40, 0x6afe10, 0x6afd90, 0x6ad030 and 0x6ad090; the names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6afe40 (slot 1, thiscall): with a cursor (+4, read once): the full byte (+0x10) set when it is 0 and the cursor is at the end
// (+8); then a 0 byte at the cursor while the full byte is 0, else over the byte before it; the cursor cleared. Then a holder (+0xc)
// is cleared and released through its slot 0.
void __fastcall NarrowWriter_Finish(u8 *writer);

// 0x6afd40 (slot 2, thiscall (character)): the full byte set when it is 0 and the cursor is at the end; while it is 0 the character's
// low byte stored at the cursor and, when its high byte is not 0, that byte after it (sic: past the end if the low one filled it).
void __fastcall NarrowWriter_PutChar(u8 *writer, void *, u32 character);

// 0x6afe10 (slot 4, thiscall (character)): with a range table, the character looked up there (Ranges_LookupAdapter) and the result
// written through slot 2.
void __fastcall NarrowWriter_PutWideChar(u8 *writer, void *, u32 character);

// 0x6afd90 (slot 5, thiscall (text, count)): with a range table (read again for each), each 16-bit character looked up there and
// written through slot 2: count -1 up to the terminating 0, else count characters.
void __fastcall NarrowWriter_PutWide(u8 *writer, void *, const u16 *text, s32 count);

// 0x6ad030 (thiscall): the vtable set (0x70a090), finished (NarrowWriter_Finish, called directly), the range table (+0xc: left 0 by that) released
// through its slot 0 when there is one, then the base vtable (0x70a0a8).
void __fastcall NarrowWriter_Dtor(u8 *writer);

// 0x6ad090 (thiscall (flags), pops 4): NarrowWriter_Dtor, freed (GameHeap_Free) for flags bit 0; returns the writer.
u8 *__fastcall NarrowWriter_DeletingDtor(u8 *writer, void *, u32 flags);

}
