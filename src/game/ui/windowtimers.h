// The GUI library's window timers: 0x20-byte nodes from a pool (0x66214f; [0xfaa630] the nodes, [0xfaa62c] their number, the byte
// +0x14 set when free) kept in a list from [0xfaa634]: the window +0, the time last fired +4, the interval +8, the id +0xc, the enabled
// flag +0x10, the next +0x18 and the previous +0x1c. Each fires by posting its window 0x10 (id, n) (0x661fe5). Names unconfirmed.
// Reimplementation of 0x661faa, 0x6620e2, 0x662306, 0x662500, 0x6624a0, 0x661e70, 0x661f2b, 0x661ece, 0x66220d, 0x6620ac, 0x662154, 0x6622d2, 0x6623ec, 0x66245c and the
// window's own 0x65ef90, 0x65f024, 0x65f077 and 0x65f0ca, and 0x661fe5 and 0x662384.
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

// 0x6624a0 (thiscall (flags), pops 4; called by 0x66247b; name unconfirmed): the vector deleting destructor: with flag 2 the array (its
// count the dword before it) unlinked last first (CRT_VectorDtorIterator, 0x20 bytes each, WindowTimer_Unlink) and its block (from the
// count) freed (Mem_Free), the pool not told; else WindowTimer_Delete's. Returns the node.
u8 *__fastcall WindowTimer_VecDelete(u8 *node, void *, u32 flags);

// 0x662306 (cdecl (window, id); name unconfirmed): the window's timer with the id killed: 0 without a list [0xfaa634] or such a node
// (WindowTimer_Find); else the list's head moves on when it is that node, the node is deleted (WindowTimer_Delete (1)) and 1 returned.
u32 __cdecl WindowTimers_Kill(u32 window, u32 id);

// 0x661f2b (cdecl (size, not read); the pool's operator new; name unconfirmed): the first free node of the pool ([0xfaa630], [0xfaa62c]
// of them; free when its dword +0x14 is set), taken (+0x14 0); 0 when none is free.
u8 *__cdecl WindowTimer_New(u32 size);

// 0x661ece (thiscall (window, interval, id), pops 12; name unconfirmed): the node's window +0, interval +8 and id +0xc, unlinked (+0x18 /
// +0x1c 0), the time +4 now (GuiClock_Now), +0x10 and +0x14 0. Returns the node.
u8 *__fastcall WindowTimer_Ctor(u8 *node, void *, u32 window, u32 interval, u32 id);

// 0x661e70 (thiscall; the pool's element constructor, pushed by 0x6621a6; name unconfirmed): a free node: the window +0, interval +8,
// id +0xc and links +0x18 / +0x1c 0, the time +4 now (GuiClock_Now), +0x10 0 and the free flag +0x14 1. Returns the node.
u8 *__fastcall WindowTimer_PoolCtor(u8 *node);

// 0x66220d (cdecl (window, interval, id); name unconfirmed): a node from the pool (WindowTimer_New) built (WindowTimer_Ctor) and put at
// the end of the list [0xfaa634] (its head when the list is empty). Returns the node, 0 when the pool has none free.
u8 *__cdecl WindowTimers_Add(u32 window, u32 interval, u32 id);

// 0x6620ac (thiscall; name unconfirmed): the node enabled (+0x10 = 1) and its time +4 now (GuiClock_Now), which it returns.
u32 __fastcall WindowTimer_Start(u8 *node);

// The window's four timers, the nodes at +0x120 + 4 * index. Each takes an id: one with both bits 0x80080 set less 0x80080 is the index,
// any other is the index as it is (sic: unchecked, as a large one reads past the window).
// 0x65ef90 (thiscall (id, interval), pops 8; name unconfirmed): a node there given the interval (Fn_6620cc); else a new one
// (WindowTimers_Add (window, interval, index + 0x80080)) put there. 1 in AX when there is a node there now, else 0 (the rest of EAX the
// index's).
u32 __fastcall Window_SetTimer(u8 *window, void *, u32 id, u32 interval);

// 0x65f024 (thiscall (id), pops 4; name unconfirmed): 0 in AX without a node there (the rest of EAX the window's); else it is started
// (WindowTimer_Start) and 1 in AX.
u32 __fastcall Window_StartTimer(u8 *window, void *, u32 id);

// 0x65f077 (thiscall (id), pops 4; name unconfirmed): as Window_StartTimer, the node stopped instead (Fn_662097: +0x10 = 0).
u32 __fastcall Window_StopTimer(u8 *window, void *, u32 id);

// 0x65f0ca (thiscall (id), pops 4; name unconfirmed): as Window_StopTimer, the timer killed instead (WindowTimers_Kill(window, the id
// as indexed plus 0x80080)): Kill's EAX; 0 in AX without a node (EAX's upper half the window's).
u32 __fastcall Window_KillTimer(u8 *window, void *, u32 id);

// 0x661fe5 (thiscall; name unconfirmed): the node's due firings: with the time since +4 (GuiClock_Now) at least the interval +8 (signed),
// +4 now (the clock read again), then for each whole interval in that time, with the node enabled (+0x10), a window +0 and [0x77c488]
// set, the window posted 0x10 (the id +0xc, the count posted so far) (Window_PostMessage); +4 then moved back by what is left. Returns the
// count posted.
s32 __fastcall WindowTimer_Fire(u8 *node);

// 0x662384 (cdecl (node); name unconfirmed): 0 without a list [0xfaa634] or a node; else the list's head moves on (+0x18) when it is the
// node, the node is deleted (WindowTimer_Delete (1)) and 1 returned.
u32 __cdecl WindowTimers_Remove(u8 *node);

// 0x662154 (cdecl (count); name unconfirmed): the pool made: the list [0xfaa634] emptied, a block of count 0x20-byte nodes (Mem_Alloc of
// count * 0x20 + 4, the count first) built (CRT_VectorCtorIterator with WindowTimer_PoolCtor / WindowTimer_Unlink) and kept at [0xfaa630]
// (0 when the allocation fails), the count at [0xfaa62c] and [0xfaa638] cleared. Returns the count.
u32 __cdecl WindowTimers_Init(u32 count);

// 0x6622d2 (cdecl; name unconfirmed): unless [0xfaa638] is set (the timers paused), each node of the list [0xfaa634] fired in turn
// (WindowTimer_Fire; the next +0x18 read after). EAX not kept.
void __cdecl WindowTimers_FireAll();

// 0x6623ec (cdecl (window, id); name unconfirmed): the window's timer with the id (WindowTimer_Find from the list's head [0xfaa634]); 0
// without a list.
u8 *__cdecl WindowTimers_Find(u32 window, u32 id);

// 0x66245c (cdecl; name unconfirmed): the pool [0xfaa630] (when there is one) deleted as an array (WindowTimer_VecDelete (3): the nodes
// unlinked, the block freed) and the list [0xfaa634] emptied ([0xfaa630] left as it was, sic). EAX not kept.
void __cdecl WindowTimers_Shutdown();
}
