// A chunked file (the TP2M map files read at 0x5192d0): +0 vtable (0x700ab0), +4 a list of chunk handlers (nodes {handler, next}, pushed by
// 0x4d3850), +0x10 the magic its files start with. A handler is an object whose slot 0 (tag) says whether it takes a chunk and slot 1
// (data, length) reads one. Reimplementation of 0x4d3860.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d3860 (thiscall (stream, name), pops 8; name unconfirmed): the stream's size (slot 5); -1: error 3 (GameError_Set of 0x785de8, arg 0,
// the name), the stream closed (slot 1), 0. A block of that size (Mem_GlobalAlloc); none: the stream closed, error 8, 0. The stream read into
// it (slot 2 (block, size)); short: error 3, closed, the block freed (Mem_GlobalFree), 0. Then closed; a size below 0x2c (signed) or a first
// dword other than the magic +0x10: freed, 0. Then the chunks from +0x24, each a tag, a length and its data, while the chunk's offset is at
// most size - 8 (signed): one whose end lies past the size (unsigned offset) or the tag "END " of length 0 ends the walk; otherwise every
// handler (the list +4, each one's next read before its calls) is asked (slot 0 (tag), any non-zero yes) and each taker reads it (slot 1
// (data, length)), and the walk goes on at its end (a negative length goes back: sic). The block freed, 1. (Debug_Printf notes left out.)
u32 __fastcall ChunkFile_ReadStream(u8 *file, void *, u8 *stream, const char *name);

// 0x4d3a60 (thiscall (name), pops 4; called by 0x519340; name invented): the file named opened (VFS_Open; the game pushes 0x74cfb8 too);
// none: error 3 (GameError_Set of 0x785de8, arg 0, the name), 0; else ChunkFile_ReadStream(the stream, no name)'s answer.
u32 __fastcall ChunkFile_Open(u8 *file, void *, const char *name);
}
