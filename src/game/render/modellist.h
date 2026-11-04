// A list of models' material groups put in drawing order and drawn.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

extern "C" {

// Its calls go by the game's addresses; a table of replacements (tests/fncmp/modellist.py gives them recorders).
void ModelList_SetCalls(const AddrCall *table, u32 count);

// 0x581db0 (thiscall, no arguments; name invented, unconfirmed): the list is a count, then that many entries (0x14 bytes, the model at +4, flags at +8) walked from the last. Each model's groups (u16 +0x5a of 0x10 bytes from +0x6c), from the last: a material (+0: the group's first dword, +2 a mip index only with
// bit 6 of 0x8bd550, +4 its texture set) gives a texture (TexSet_Texture, 0 without). A texture with flag 0x1000 keys a binary tree of up to 4096 nodes by its +0x1c (0 without, or with none); greater keys one way, lesser the other. With a texture the group sorts by TexLoad_DriverFlags, batching when that shares a bit with [0x879ab0]+0xc and the entry lacks 0x2000000;
// without, it has the order 0x1256 and batches. A node's batching groups stay in order of that value (signed), then the texture (unsigned); the others go at the front of a second list. Then in the nodes' creation order: batching groups added to one Model_BeginBatch (a null batch ends it all with 1, sic) until the order changes (Batch_Close); others each with their own.
// A failing add or close (or begin, for the others): 0. Returns 1.
u32 __fastcall ModelList_Draw(const u8 *list);
}
