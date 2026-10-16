// Taking an entry off a free list of 0x34-byte records: reimplementation of 0x520d60. Named by what it does until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x520d60 (cdecl; index): -1 when the free list's head (the signed word at 0x816d24) is negative. Otherwise the head becomes the word at +2 (0x140-byte records from
// 0x80cf00) of record `index`, and the records at [0x80cef8] (0x34 bytes each; the words at +0x30 next and +0x32 previous, -1 for none) are relinked: the new head's
// previous is 0xffff, the taken one's next is the record's old word and that one's previous is the taken one. Returns the taken index.
s32 __cdecl FreeList_Take(s32 index);

}  // extern "C"
