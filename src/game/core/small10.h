// A ring of words, a lookup table, a kind check, the 7028xx constructors, list erases and two small table writers: reimplementation of 0x4caa70,
// 0x59c320, 0x535600, 0x5bce60, 0x5bd2a0, 0x5bd6b0, 0x5cc700, 0x5cc830, 0x5cdf70, 0x5e7840 and 0x5ed700.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4caa70 (thiscall; out, which, index, pops 12): the ring of words at the record 20 * (which + 0x18d2) bytes in (head at +8, capacity at +0xc,
// a wrapped flag at +0x10, the words from +4): the word `index` places before the head (counting on from the end for a wrapped ring), or 0
// for none (index >= capacity, or past the head of a ring that has not wrapped), is stored through out. Returns out.
u16 *__fastcall Ring16_Get(u8 *object, void *, u16 *out, s32 which, s32 index);

// 0x59c320 (stdcall; id): the byte at +0x20 of the 56-byte record at 0x76e300 + 56 * id when its id (+4) matches; 0 for 0x15f and 0x160 and for
// ids that match no record (searched from 0x76e304 on, but a match found by the search gives the byte of the record at id's own slot: sic).
u8 __stdcall Table76e300_GetByte(u32 id);

// 0x535600 (thiscall; kind, pops 4): whether the kind is allowed beside the kind in the dword at +8 (see the code: 0 always, 4 only
// against anything but 4, 0x15 and 1 with their partners, equal kinds, 0 as the object's kind, 4 against 1, and 3 against 1 only while the
// dword at 0x820ab8 is 0).
u32 __fastcall Kinds_Compatible(const u8 *object, void *, u32 kind);

// 0x5bce60 / 0x5bd2a0 / 0x5bd6b0 (thiscall): like Class702880_Ctor (see small9) but the vtable pointer at +0 is 0x7028d0 / 0x7028f8 / 0x702920
// while the 16-byte member (vtable 0x7028c8 / 0x7028f0 / 0x702918) is made and 0x7028b8 / 0x7028e0 / 0x702908 after it.
u8 *__fastcall Class7028b8_Ctor(u8 *object);
u8 *__fastcall Class7028e0_Ctor(u8 *object);
u8 *__fastcall Class702908_Ctor(u8 *object);

// 0x5cc700 / 0x5cc830 (thiscall): the nodes of the circular list whose sentinel is at +0x68 / +0x74 (count at +0x6c / +0x78) whose dword at +8
// equals the dword at +8 of the record the dword at +0x7c points to are unlinked and freed, the count one less each.
void __fastcall List68_EraseMatching(u8 *object);
void __fastcall List74_EraseMatching(u8 *object);

// 0x5cdf70 (thiscall; out, node, pops 8): the node is unlinked from its circular list, the counted buffer (see small6) at its +0xc released
// and its fields +0xc, +0x10 and +0x14 cleared, the node freed, the count at +8 one less, and the node that followed it stored through out.
// Returns out.
u32 *__fastcall List_EraseNodeCounted(u8 *object, void *, u32 *out, u8 *node);

// 0x5e7840 (thiscall; a, b, c, pops 12): the block at +0xc is freed (when there is one), a new one of a * b * c bytes cleared; a, b, c at
// +0, +4, +8 (not when the block could not be made). Returns 1, or 0 when there is no block.
u8 __fastcall Array3D_Realloc(u8 *object, void *, u32 a, u32 b, u32 c);

// 0x5ed700 (cdecl; index, shift, a, b, c): the 68-byte record at 0xf86918 + 68 * index: with shift set the dwords at +4 and +8 = a, b;
// otherwise they get those at +0xc and +0x10; then +0xc = a, +0x10 = b and +0 = c.
void __cdecl KeyRecord_Push(u32 index, u32 shift, u32 a, u32 b, u32 c);

}  // extern "C"
