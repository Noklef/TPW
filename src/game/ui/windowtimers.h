// The GUI library's window timers: 0x20-byte nodes from a pool (0x66214f; [0xfaa630] the nodes, [0xfaa62c] their number, the byte
// +0x14 set when free) kept in a list from [0xfaa634]: the window +0, the time last fired +4, the interval +8, the id +0xc, the enabled
// flag +0x10, the next +0x18 and the previous +0x1c. Each fires by posting its window 0x10 (id, n) (0x661fe5). Names unconfirmed.
// Reimplementation of 0x661faa, 0x6620e2, 0x662306 and 0x662500.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x661faa (thiscall; also the pool's element destructor; name unconfirmed): the node unlinked: the previous one's next set to its next and
// the next one's previous to its previous (each when set). EAX is left as its previous when it has a next, else the node.
u8 *__fastcall WindowTimer_Unlink(u8 *node);

// 0x6620e2 (thiscall (window, id), pops 8; name unconfirmed): the last node from this one on (by +0x18, searched recursively from the
// end) whose window and id are the ones given; 0 when none is.
u8 *__fastcall WindowTimer_Find(u8 *node, void *, u32 window, u32 id);

// 0x662500 (thiscall (flags), pops 4; name unconfirmed): the scalar deleting destructor: unlinked (WindowTimer_Unlink), then with flag 1
// given back to the pool (Fn_661f9b: its byte +0x14 set). Returns the node.
u8 *__fastcall WindowTimer_Delete(u8 *node, void *, u32 flags);

// 0x662306 (cdecl (window, id); name unconfirmed): the window's timer with the id killed: 0 without a list [0xfaa634] or such a node
// (WindowTimer_Find); else the list's head moves on when it is that node, the node is deleted (WindowTimer_Delete (1)) and 1 returned.
u32 __cdecl WindowTimers_Kill(u32 window, u32 id);

// 0x661f2b (cdecl (size, not read); the pool's operator new; name unconfirmed): the first free node of the pool ([0xfaa630], [0xfaa62c]
// of them; free when its dword +0x14 is set), taken (+0x14 0); 0 when none is free.
u8 *__cdecl WindowTimer_New(u32 size);

// 0x661ece (thiscall (window, interval, id), pops 12; name unconfirmed): the node's window +0, interval +8 and id +0xc, unlinked (+0x18 /
// +0x1c 0), the time +4 now (GuiClock_Now), +0x10 and +0x14 0. Returns the node.
u8 *__fastcall WindowTimer_Ctor(u8 *node, void *, u32 window, u32 interval, u32 id);

// 0x66220d (cdecl (window, interval, id); name unconfirmed): a node from the pool (WindowTimer_New) built (WindowTimer_Ctor) and put at
// the end of the list [0xfaa634] (its head when the list is empty). Returns the node, 0 when the pool has none free.
u8 *__cdecl WindowTimers_Add(u32 window, u32 interval, u32 id);
}
