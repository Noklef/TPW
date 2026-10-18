// The mesh draws: the list at [0x8bcbb8] (0x14004 bytes, CreateGlobal8bcbb8_5804b0 / FreeGlobal8bcbb8_580500 in core/globals) of
// meshes transformed for drawing, a count then 0x1000 entries of 0x14 bytes: {the renderer's ([0x87a6d0]) dword +0x30, the mesh, the
// flags, the transformed vertices (in the draw buffer's scratch area), the faces}. Filled while the scene is walked (0x57a6d6).
// Reimplementation of 0x581cd0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x581cd0 (thiscall (list; matrix, b, c, mesh, flags), b and c unused; name unconfirmed): the mesh added to the list. With 0x1000 entries
// already (signed): the engine error (1, 0xb0754008) pushed (EngineErrors_Push on 0x879ad0), 0. A mesh with no vertices (the word at
// +0x58) adds nothing: 1. Else the entry after the count gets the renderer's +0x30, the mesh and flags | [0x8bb624]; its vertices'
// room (0x10 bytes each) from the draw buffer ([0x8bcbc4], DrawBuffer_Alloc): none, 1 with the count left alone (the entry's first three
// dwords written, sic). Then the vertex transformer's ([0x87b1d8]) slot 2 called cdecl as (transformer, matrix, room, the mesh's vertices
// +0x60, 0x10, 0, the count), the entry's faces are the mesh's +0x64 with mesh flag 0x1000000, else its +0x9c plus 4, the count (read
// again: a count of -1 puts the entry before the list, its faces dword over the count, sic) grows: 1.
u32 __fastcall MeshDraws_Add(u8 *list, void *, u32 matrix, u32 b, u32 c, u8 *mesh, u32 flags);

}
