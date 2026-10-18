// The GUI library's message queue (the objects at [0xfaa5f4] and [0xfaa5f0]): +0 the entries (16 bytes each: window, message, a, b; a
// free entry has window 0), +4 the capacity, +8 the next to take, +0xc the next to fill (both counting down, wrapping to the last), +0x10
// the count. Reimplementation of 0x669735 and 0x6697e3.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x669735 (thiscall; name unconfirmed): the entries (when set) freed (Mem_Free) and +0 cleared.
void __fastcall MsgQueue_Free(u8 *queue);

// 0x6697e3 (thiscall (window, message, a, b), pops 16; name unconfirmed): when the entry to fill is free and window is not 0, the four
// are stored there, the count goes up and the fill index down (below 0: the capacity - 1); 1 in AX. Otherwise 0 in AX. Only AX is set.
u32 __fastcall MsgQueue_Push(u8 *queue, void *, u32 window, u32 message, u32 a, u32 b);
}
