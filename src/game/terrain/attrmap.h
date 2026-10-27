// The attribute map's MAP chunk handler (vtable 0x700aa0: slot 2 the tag test, slot 3 the read): reimplementation of 0x519390 and
// 0x4d8cf0 (thiscall). The handler's +4 is its chunk tag, its +8 the map cells (0x44 bytes, 128 to a row, cell id 1 the first).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x519390 (tag): 1 when the handler's tag +4 is the tag asked for, else 0.
u32 __fastcall AttrMapHandler_MatchTag(u8 *handler, void *, u32 tag);

// 0x4d8cf0 (chunk, unused): the chunk's w x h bytes (+0 width, +4 height, signed; the bytes from +0x1c, row by row) into the map
// cells' status bytes +0x26: row r and column c land on the cell at x = r - [0x785584], y = c - [0x785588], those outside the map's
// 128 x 128 skipped (their bytes still passed over).
void __fastcall AttrMap_ReadMapChunk(u8 *handler, void *, const u8 *chunk, u32 unused);

// 0x5192d0 (thiscall (name), pops 4; an SEH frame): a TP2M chunk file on the stack (vtable 0x700ab0, its handler list +4 head, +8
// tail, +0xc count empty, the magic "TP2M" at +0x10) given one handler (vtable 0x700aa8: the tag "MAP ", +8 the cells, this) on its
// list (List_PushHeadForward), then ChunkFile_Open of the name; the file destroyed (its vtable 0x700aa0, the list's nodes freed).
// Returns Open's answer.
u32 __fastcall TP2M_ReadChunks(u8 *cells, void *, const char *name);
}
