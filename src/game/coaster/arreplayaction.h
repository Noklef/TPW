// The two actions behind the coaster's action replay (ArReplay_Step, coaster/arreplay): 0x404d80, 0x404f70.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

extern "C" {

// Their calls go by the game's addresses; a table of replacements so that they can run outside the game (tests/fncmp/arreplayaction.py gives every one a recorder).
void ArReplayAction_SetEnv(const AddrCall *table, u32 count);

// 0x404d80 (thiscall (action, word), pops 8; self unused; name invented, unconfirmed). id is the word's low 16 bits. Action 0 GameMode_Set (id, 0), 0xa Global81d7a4_SetRecorded (id, 0), 0x19 GameMode_SetOnly (id, 0),
// 0x1e Global81ecac_SetRecorded, 0x23 Global81b0cc_SetRecorded (id), 0x3d, 0x3e Car_SwapWithNextAt, Car_SwapWithPrevAt (the whole word), 0x42 CoasterEdit_SetEditingById, 0x43 0x447d60, 0x44 CoasterEdit_SetEditingAt (id);
// 0x32: the first thing of kind 3 in the chain of cell id (head at the world's +0x2b8, 0x44 each; a thing's +0xa the next, 0x7cfb90's table, 20 bytes each) is selected and the focus cell updated; none: nothing. Any other: nothing.
void __fastcall ArReplay_Action(u8 *self, void *, u32 action, u32 word);

// 0x404f70 (thiscall (action, word, n, words), pops 0x10; self unused; name invented, unconfirmed). Actions 0x50 and 0x55 only. A wide string of the n & 0xff words is checked by the save singleton 0xf7c58c (made if none; 0 if that fails).
// The thing: word's low 16 bits 0xea60: the table's entry of the world's +0x1da732; or the first of kind 3 in that cell's chain, else its last (none: 0). Then for 0x50 the two calls 0x4e2950, 0x4e2920 (a buffer of 0x64 words) and 0x4682b0 (thing, text, buffer);
// for 0x55 0x4e2a50, 0x4e2860 and 0x4682b0 (thing, buffer, text). An empty text is 0x6fd6c4.
void __fastcall ArReplay_TextAction(u8 *self, void *, u32 action, u32 word, u32 n, const u16 *words);
}
