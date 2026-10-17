// Sound voices and their free list: starting a voice from a template, scaling its levels, taking and returning records. Reimplementation of 0x520d60, 0x521890, 0x521d60, 0x522360, 0x5224f0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x522360 (cdecl; template voice, x, y, z): 0xffffffff without the sound system (0x816d40) or for a voice outside 0 to 0x13 or with no free instance (the signed word at
// 0x816d22 negative); 0 when the flag 0x80ced8 is set and the template's byte at +0x44 is not. Otherwise the instance at the free head (0x68-byte records from 0x8164fc,
// links as words at +0x60 next / +0x62 previous) moves to the head of the active list (0x816d1e), takes the template's 0x68 bytes (its links kept), a use flag of 1 at +0, the
// position x, y, z >> 4 at +0xc, +0x10, +0x14 and the counter at 0x80ceea (then incremented) at +8, and 0 at +0xa. Returns the counter in the high half, the index in the low.
u32 __cdecl Voice_Start(s32 voice, s32 x, s32 y, s32 z);

// 0x520d60 (cdecl; index): -1 when the free list's head (the signed word at 0x816d24) is negative. Otherwise the head becomes the word at +2 (0x140-byte records from
// 0x80cf00) of record `index`, and the records at [0x80cef8] (0x34 bytes each; the words at +0x30 next and +0x32 previous, -1 for none) are relinked: the new head's
// previous is 0xffff, the taken one's next is the record's old word and that one's previous is the taken one. Returns the taken index.
s32 __cdecl FreeList_Take(s32 index);

// 0x521890 (cdecl; record, index): nothing for a negative record. The record leaves its list (the table entry `index` has the first of its list as the word at +2: it
// becomes the record's next) and goes to the head of the free list (its previous 0xffff, its next the old head, the word at +0x24 cleared; the head word at 0x816d24 is the record).
void __cdecl FreeList_Return(s32 record, s32 index);

// 0x521d60 (cdecl; index): the 0x140-byte entry at 0x80cefc + index * 0x140 has its four signed bytes at +0x68, its word at +0x64 and its dword at +0x60 (the last two only
// when above 0) scaled by the factor at 0x80cef0 / 1024 (the bytes limited to 0x7f above; a result of 0 becomes 1). Returns the entry.
u8 *__cdecl Voice_ScaleLevels(s32 index);

// 0x5224f0 (cdecl; index, reset; name unconfirmed): the entry at 0x80cefc + index * 0x140. A reset sets its word at +0 to 1 and the spread (+0xcc) to 0. Otherwise the
// spread grows by the signed word at +0x50 and, while the word at +0 is above 0, records are taken (FreeList_Take) for as long as the loop count stays below the signed byte
// count (+0x6b, +0x6a, +0x69 for the dword at +0x20 * 4 / the dword at +0xd4 of 0, 1, 2; else and for a divisor of 0 +0x68) times (spread >> 7) + 0x20 (a failed take
// still counts). Each one bumps 0x80ced0 and the word at +8 and gets the index, kind 2, the words at +0x74 and +4 (signed byte), the dword at +0x114, a life (+0x20 and
// +0x1c, the latter at least 1) of the dword at +0x78 plus a random remainder by half of it, and a position and velocity from a random angle of the sines at [0x80ceec]
// (both random numbers from the generator at 0x80cee0). The spread is stored back.
void __cdecl Emitter_Spawn(s32 index, u32 reset);
}  // extern "C"
