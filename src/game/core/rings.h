// Rings: the element a number of places from the head or the end of a ring, pushing without repeats and splicing nodes. Reimplementation of 0x4caa70, 0x50c100, 0x50c190, 0x5d0a50. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4caa70 (thiscall; out, which, index, pops 12): the ring of words at the record 20 * (which + 0x18d2) bytes in (head at +8, capacity at +0xc,
// a wrapped flag at +0x10, the words from +4): the word `index` places before the head (counting on from the end for a wrapped ring), or 0
// for none (index >= capacity, or past the head of a ring that has not wrapped), is stored through out. Returns out.
u16 *__fastcall Ring16_Get(u8 *object, void *, u16 *out, s32 which, s32 index);

// 0x50c100 (thiscall; id, value, pops 8): the ring of 32 word pairs from +4 (the next position at +0x88): unless the last pushed pair has the same id
// and value, the pair (low 16 bits of id, low 16 bits of value) is stored at the position, which moves on (modulo 32).
void __fastcall Ring_PushUnique(u8 *object, void *, u32 id, u32 value);

// 0x50c190 (thiscall; index, pops 4): in the ring of dwords from +4 (count at +0x88, 32 are held at +4, the others from +0x84 on): the
// address of the element index from the end: +4 + (count - index) * 4 when index <= count, otherwise +0x84 + (count - index) * 4
// (wrapped). The original reports index >= 0x20 to its empty debug printf.
u8 *__fastcall Ring_FromEnd(u8 *object, void *, u32 index);

// 0x5d0a50 (stdcall; position, unused, first, last, pops 16): the nodes of a doubly linked ring ({next, prev}) from first up to (not
// including) last are moved to before position (a splice); the links are rewritten in the original's order. Returns first (EAX).
u32 __stdcall Ring_Splice(u8 *position, u32 unused, u8 *first, u8 *last);
}  // extern "C"
