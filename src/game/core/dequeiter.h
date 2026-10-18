// A deque of dwords' iterator (blocks of 0x1000 bytes, a map of block pointers): reimplementation
// of 0x632200, 0x632230, 0x632290, 0x632310, and the constructor 0x632380 of an object nearby.
//
// The iterator is four dwords: +0 the block's first byte, +4 its end (first + 0x1000), +8 the
// current position and +0xc the block's slot in the map.
#pragma once

#include "../../../re/types.h"

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
}
