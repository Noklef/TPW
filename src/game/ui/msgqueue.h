// The GUI library's message queue (the objects at [0xfaa5f4] and [0xfaa5f0]): +0 the entries (16 bytes each: window, message, a, b; a
// free entry has window 0), +4 the capacity, +8 the next to take, +0xc the next to fill (both counting down, wrapping to the last), +0x10
// the count. Reimplementation of 0x669735, 0x6697e3,
// 0x6698a4 and 0x6698e6.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x669735 (thiscall; name unconfirmed): the entries (when set) freed (Mem_Free) and +0 cleared.
void __fastcall MsgQueue_Free(u8 *queue);

// 0x6697e3 (thiscall (window, message, a, b), pops 16; name unconfirmed): when the entry to fill is free and window is not 0, the four
// are stored there, the count goes up and the fill index down (below 0: the capacity - 1); 1 in AX. Otherwise 0 in AX. Only AX is set.
u32 __fastcall MsgQueue_Push(u8 *queue, void *, u32 window, u32 message, u32 a, u32 b);

// 0x6698a4 (cdecl (pairs, a, b: words); name unconfirmed): 1 in AX when the list of word pairs (ended by a pair whose second is -1)
// holds (a, b), else 0. Only AX is set.
u32 __cdecl WordPairs_Contains(const s16 *pairs, u32 a, u32 b);

// 0x6698e6 (thiscall; name unconfirmed): takes the entry +8: 0 in AX without entries or when it is free. An entry with any of message,
// a and b 0x80808080 is dropped unsent. Messages 0x1000a-0x1000c go to the first window that takes them: [0xfaa57c] when the pairs
// [0xfaa578] (WordPairs_Contains) hold (a's low word, b's third byte), then [0xfaa5ec], then [0xfaa5dc] (each set and with bit 0 of
// Fn_65d57c), else nowhere; the others to the entry's window (Window_CallHandler). Then the entry is freed, the count goes down and the
// take index down (below 0: the capacity - 1); 1 in AX.
u32 __fastcall MsgQueue_DispatchOne(u8 *queue);
}
