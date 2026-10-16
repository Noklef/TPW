// Wrappers and clears over the counted buffers and vectors: reimplementation of 0x47db80, 0x4d3850, 0x47bfe0, 0x4f04d0 and 0x5c9970.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x47db80 (thiscall; flags, pops 4): CountedVector14_Clear (small9); when bit 0 of the flags is set the object is freed. Returns the object.
u8 *__fastcall CountedVector14_Deleter(u8 *object, void *, u32 flags);

// 0x4d3850 (thiscall; item, pops 4): List_PushHead (small9) of the same object.
u32 __fastcall List_PushHeadForward(u8 *list, void *, u32 item);

// 0x47bfe0 (stdcall; first, last, pops 8): CountedVector14_Clear of each 0x24-byte element from first up to last.
void __stdcall CountedVector14_ClearRange(u8 *first, u8 *last);

// 0x4f04d0 (thiscall): every node of the circular list at +4 (count at +8) is unlinked, CountedBuf_ReleaseAndClear (small8) of its +8 is done and
// the node freed; then the sentinel is freed and the dwords at +4 and +8 = 0.
void __fastcall CountedList_ClearRelease(u8 *object);

// 0x5c9970 (thiscall): the block at +0x20 is freed (when there is one) and the pointer cleared; the counted buffers at +0x14 and +4 (the count is the
// block's first byte: freed 1 byte below the pointer) are released and their three-dword fields cleared.
void __fastcall Counted2_Clear(u8 *object);

}  // extern "C"
