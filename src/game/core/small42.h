// Taking an entry off a free list of 0x34-byte records: reimplementation of 0x520d60. Named by what it does until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

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

}  // extern "C"
