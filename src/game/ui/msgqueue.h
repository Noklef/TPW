// The GUI library's message queue (the objects at [0xfaa5f4] and [0xfaa5f0]): +0 the entries (16 bytes each: window, message, a, b; a
// free entry has window 0), +4 the capacity, +8 the next to take, +0xc the next to fill (both counting down, wrapping to the last), +0x10
// the count. Reimplementation of 0x6596b0, 0x669735, 0x669767, 0x6697e3, 0x6698a4, 0x6698e6, 0x669ad7, 0x669b3e and 0x669bdb.
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
// 0x669767 (thiscall (capacity), pops 4; name unconfirmed): 0 in AX when the entries are set or their allocation (Mem_Alloc,
// capacity * 16 bytes) fails. Otherwise both indices the capacity - 1, the capacity stored, the count 0 and the entries zeroed (memset);
// 1 in AX. Only AX is set.
u32 __fastcall MsgQueue_Init(u8 *queue, void *, u32 capacity);

// 0x669ad7 (thiscall; name unconfirmed): the entry to take when there are entries, it is not free and none of its message, a and b is
// 0x80808080; else 0.
u32 *__fastcall MsgQueue_Peek(u8 *queue);

// 0x669b3e (thiscall (window), pops 4; name unconfirmed): each of the capacity's entries (free ones too) whose window is the one given
// gets message, a and b 0x80808080 (dropped when taken); their number in AX, -1 without entries. Only AX is set.
u32 __fastcall MsgQueue_Cancel(u8 *queue, void *, u32 window);

// 0x669bdb (thiscall; name unconfirmed): with entries, both indices the capacity - 1, the count 0 and the entries zeroed (memset).
void __fastcall MsgQueue_Clear(u8 *queue);

// 0x658e42 (cdecl (most); name unconfirmed): the posted queue [0xfaa5f0]'s messages sent (MsgQueue_DispatchOne) until none is left or
// `most` were counted; the count (0 without the queue). Without the hook [0xfaa5fc] (sic) one more is taken once `most` is reached (the
// take comes before the check), uncounted; with it, each is first shown to the hook (the entry's window, message, a and b; MsgQueue_Peek,
// none for a dropped one), the hook read again each time.
s32 __cdecl MsgQueue_Pump(s32 most);

// 0x658f0b (cdecl (most); called by 0x48a6bc; name unconfirmed): with the second queue [0xfaa5f4] (read again for each), its messages
// dispatched (MsgQueue_DispatchOne) while one is (AX) and fewer than `most` were counted; the count. The dispatch comes before the check
// (sic): one more is taken once `most` is reached, uncounted.
s32 __cdecl MsgQueue_PumpSecond(s32 most);

// 0x6596b0 (thiscall (flags), pops 4; name unconfirmed): the scalar deleting destructor: MsgQueue_Free, then with flag 1 the queue's block freed
// (Mem_Free). Returns the queue.
u8 *__fastcall MsgQueue_Delete(u8 *queue, void *, u32 flags);
}
