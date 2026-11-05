// A deque of dwords' iterator (blocks of 0x1000 bytes, a map of block pointers): reimplementation
// of 0x632150, 0x632200, 0x632230, 0x632290, 0x632310, and the constructor 0x632380 of an object nearby; the
// sound code's deque of dwords' members (Deque32_*, 0x6c1550-0x6c3010) below.
//
// The iterator is four dwords: +0 the block's first byte, +4 its end (first + 0x1000), +8 the
// current position and +0xc the block's slot in the map.
#pragma once

#include "../../../re/types.h"

struct AddrCall;
void CommandQueue_SetCalls(const AddrCall *table, u32 count);   // CommandQueue_OpenBox's Gui_TopDepth, Dialog_Box and Window_SetHandler (the bench's recorders)

extern "C" {

// 0x632200 (thiscall; cur, node, pops 8): the block *node, position cur. Returns the iterator.
u32 *__fastcall DequeIter_Set(u32 *it, void *, u32 cur, const u32 *node);

// 0x632230 (thiscall; out, n, pops 8): the iterator n dwords on, in out (returned): across blocks,
// walking the map (n counted from the current position, negative backwards).
u32 *__fastcall DequeIter_Plus(const u32 *it, void *, u32 *out, s32 n);

// 0x632290 (thiscall; other, pops 4): this - other in dwords: the plain difference within a block,
// else counted through the blocks.
s32 __fastcall DequeIter_Diff(const u32 *it, void *, const u32 *other);

// 0x632310 (thiscall): one back, moving to the end of the previous block (the map slot - 4) from
// a block's start. Returns the iterator.
u32 *__fastcall DequeIter_Prev(u32 *it);

// 0x632380 (thiscall): an object with vtable 0x703c14, +0x10 = 1, +0x14-+0x1c, +0x28-+0x30,
// +0x38-+0x40 = 0 and the bytes +0x24 and +0x34 = an uninitialised local, which is the top byte of
// the object's own address (sic). Returns the object.
u8 *__fastcall Class703c14_Ctor(u8 *object);

// 0x47f7d0 (cdecl; name unconfirmed): the command queue's (0x7b4ba8, see CommandQueue_Post) first element shown and taken off, inside its critical
// section (0x7b4bd8): its text (record table the element's first dword, index its second: Records_DataIfTerminated, core/methods) widened
// (Language_Widen, core/language) to the object at [0x7b4ba0]'s slot 9 (thiscall, one argument); then the cursor moves 8 on and the count falls,
// and at the block's end or the last element the block is released as Deque8_Clear does. 1 when the queue was empty (nothing done), else 0.
u32 __cdecl CommandQueue_ShowNext();

// 0x47f620 (cdecl; name invented, unconfirmed): within the section, with an element queued, no box yet ([0x7b4ba0] 0) and Gui_TopDepth (0, 0) below 0x23: the first element taken (as CommandQueue_ShowNext), its text widened into a
// record (text; bytes 0xff x 3, the fourth not written; 4; 0) for Dialog_Box (record, 2, 0x47f7d0, 0 x 5), whose window goes to [0x7b4ba0] and takes the handler 0x47f7a0 (Window_SetHandler).
void __cdecl CommandQueue_OpenBox();

// 0x47f340 (thiscall; name unconfirmed): empties a deque of 8-byte elements in blocks of 0x1000 bytes (the begin iterator at +4 {first, end, cursor, map slot}, the end iterator at +0x14, the
// map at +0x24, the count at +0x2c): while the count is not 0 the cursor (+0xc) moves on by 8 and the count falls; at the end of a block, or the last element, the block (the map slot's, which moves
// on) is freed and, with the count at 0, both iterators are cleared and the map freed, else the begin iterator is set to the next block.
void __fastcall Deque8_Clear(u8 *deque);

// 0x47f410 (cdecl; code, param; name unconfirmed): posts (param, code) to the global deque of 8-byte elements at 0x7b4ba8 (laid out as above,
// the map's size at +0x28) inside its critical section (0x7b4bd8, CritSec_Enter / CritSec_Leave, whose result is returned); a param of 0 is the
// dword at 0x78ba88. With elements and room in the last block the element goes at the end cursor. Else a block of 0x1000 is allocated: with
// none, a map of 2 (size 2) whose second slot it is, both iterators on it and the element in its middle (+0x800); with a slot left after the
// end's, it goes there; else a map of twice the blocks in use (bytes 8 * count, 0 when negative) with them copied (Dwords_Copy) from its
// (2 * count) / 4th slot, the old one freed, the block after them, the iterators set again (DequeIter_Set). The element (unless at 0) is
// written, the end cursor moves 8 on and the count is stepped.
u32 __cdecl CommandQueue_Post(u32 code, u32 param);

// The sound code's deques of dwords (std::deque<u32>: blocks of 0x1000 bytes; the begin iterator at +4, the end iterator at +0x14, each {first, end,
// position, map slot}; the map at +0x24, its size in slots at +0x28, the count at +0x2c), names unconfirmed. An iterator passed by value is its four
// dwords on the stack.
struct DequeIt {
    u32 first, end, cur, node;
};

// 0x6c2990 (thiscall): the map allocated for +0x28 slots (+0x24 = Mem_Alloc of 4 * the size, 0 bytes when that is negative).
void __fastcall Deque32_AllocMap(u8 *deque);
// 0x6c2910 (thiscall): the begin iterator's block freed and its slot moved on; with the count at 0 both iterators are cleared and the map freed
// (the pointer kept), else the begin iterator is set to the start of the next block.
void __fastcall Deque32_FreeFront(u8 *deque);
// 0x6c2ca0 (thiscall): the same at the back: the end iterator's block freed, its slot moved back; with the count at 0 as Deque32_FreeFront, else
// the end iterator is set to the end of the block before.
void __fastcall Deque32_FreeBack(u8 *deque);
// 0x6c1550 (thiscall): pop_front: the begin position one dword on, the count lowered; at 0, or at the block's end, Deque32_FreeFront.
void __fastcall Deque32_PopFront(u8 *deque);
// 0x632080 (thiscall): pop_front until the count is 0 (inline in the original).
void __fastcall Deque32_Clear(u8 *deque);
// 0x631ec0 (thiscall on the owner, the deque at +0x10; out, value, pops 8): when the owner's slot 0x1c answers, the object at the deque's front told of the value (its slot 0, unless 0), its +8
// cleared, pop_front (inline). out = 0; returns out. Name invented.
u32 *__fastcall DequeOwner_Pop(u8 *owner, void *, u32 *out, u32 value);
// The same owner: a queue under a lock (vtable 0x703bc8, its base's 0x703bf0; the deque at +0x10, the section at +0x40). Names invented.
// 0x631870 (thiscall): Class703770_Ctor, +8, +0xc and +0x14-+0x3c = 0, the section made. The deque's allocator byte at +0x10 is stack garbage in the original and not written (nothing reads it). Returns self.
u8 *__fastcall LockedQueue_Ctor(u8 *self);
// 0x631fc0 / 0x632020 (thiscall): the section entered / left, through a guard on the stack.
void __fastcall LockedQueue_Lock(u8 *self);
void __fastcall LockedQueue_Unlock(u8 *self);
// 0x631d10 (thiscall; value, pops 4): under the lock, push_back (with no elements or the last block full, Deque32_GrowBack, inline in the original). 0x631d00: the same, one call more.
void __fastcall LockedQueue_Push(u8 *self, void *, u32 value);
void __fastcall LockedQueue_PushThunk(u8 *self, void *, u32 value);
// 0x6319a0 (thiscall): erase(begin, end) inline: nothing before or after the stretch, so only its pop_backs do anything; the section deleted; pop_front while the count (it can only be one the iterators
// disagree with) is not 0; Class703780_DtorD's code. 0x631980 (flags, pops 4): that; with bit 0, freed. Returns self.
void __fastcall LockedQueue_Dtor(u8 *self);
u8 *__fastcall LockedQueue_DeletingDtor(u8 *self, void *, u32 flags);
// 0x631950 (thiscall; flags, pops 4): the base's: Class703780_DtorD; with bit 0, freed. Returns self.
u8 *__fastcall LockedQueueBase_DeletingDtor(u8 *self, void *, u32 flags);
// 0x632150 (thiscall): pop_back: the end position one dword back, the count lowered; at 0, or at the block's start, the back block freed as
// Deque32_FreeBack (inline in the original).
void __fastcall Deque32_PopBack(u8 *deque);
// 0x6c1940 (thiscall): a new block for the back (0x1000 bytes, allocated first). With the count at 0 a new map of 2 slots (the old one left), the
// block in the second, both iterators at its middle (+0x800); with a slot left after the end's, the block goes there; else a map of twice the
// blocks in use (n; 8n bytes, 0 when negative) takes their slots from slot (2n) / 4 on, the old map freed, the block after them, the begin
// iterator set to its new slot (its position kept). The end iterator is then at the new block's start.
void __fastcall Deque32_GrowBack(u8 *deque);
// 0x6c3010 / 0x6c2f80 (cdecl; out, first, last, dest; iterators by value): copy and copy_backward of the dwords from first up to last (the
// positions compared) to dest on / back from dest, across blocks. *out = dest as it ends; returns out.
u32 *__cdecl Deque32_Copy(u32 *out, DequeIt first, DequeIt last, DequeIt dest);
u32 *__cdecl Deque32_CopyBackward(u32 *out, DequeIt first, DequeIt last, DequeIt dest);
// 0x6c15f0 (thiscall; out, first, last, pops 0x24): erase(first, last). With fewer elements before first than after last, the ones before move
// back up to last (DequeIter_Prev on both, until the source reaches the begin position) and the front is popped once per element erased (as
// Deque32_PopFront); else the ones from last move down to first (DequeIter_Inc, until the end position as it was) and the back is popped (the
// end position one dword back, the count lowered; at 0, or at the block's start, Deque32_FreeBack). *out = begin + the count before first
// (the begin iterator itself when 0); returns out.
u32 *__fastcall Deque32_Erase(u8 *deque, void *, u32 *out, DequeIt first, DequeIt last);
}
