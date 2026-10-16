// Counted buffer release, a three-way byte switch, a ring lookup, virtual base deleting destructors and a list splice:
// reimplementation of 0x4f75e0, 0x4fe980, 0x5012b0, 0x50c190, 0x579020, 0x5790c0, 0x579150 and 0x5d0a50.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4f75e0 (thiscall): the counted buffer at +4 (see small6), when set, is released (one use less, or the block freed when the count
// is 0 or 0xff); then the dwords at +4, +8 and +0xc = 0.
void __fastcall CountedBuf_ReleaseAndClear(u8 *object);

// 0x4fe980 (stdcall; index): the byte at 0x785058 / 0x78505c / 0x785060 for 0 / 1 / 2, otherwise 0 (the original reports that to its empty
// debug printf).
u8 __stdcall Bytes785058_Get(u32 index);

// 0x5012b0 (thiscall): the dword at +0x1f8 = 1 (the original also reads the handle at +0x1dc into its debug print).
void __fastcall Visitor_Set1f8One(u8 *object);

// 0x50c190 (thiscall; index, pops 4): in the ring of dwords from +4 (count at +0x88, 32 are held at +4, the others from +0x84 on): the
// address of the element index from the end: +4 + (count - index) * 4 when index <= count, otherwise +0x84 + (count - index) * 4
// (wrapped). The original reports index >= 0x20 to its empty debug printf.
u8 *__fastcall Ring_FromEnd(u8 *object, void *, u32 index);

// 0x579020 / 0x5790c0 / 0x579150 (thiscall on the object 8 bytes above the base; flags, pops 4): the base's (the pointer - 8) virtual base
// offset (dword +4 of its vtable) names where the vtable 0x7018d0 / 0x701920 / 0x7018f8 is stored; the dword at (the offset - 4, read
// again from the base's new vtable) = the offset - 8; the dword at +8 = 0x701940; when bit 0 of the flags is set the base is freed.
// Returns the base.
u8 *__fastcall VBase_Deleter_7018d0(u8 *object, void *, u32 flags);
u8 *__fastcall VBase_Deleter_701920(u8 *object, void *, u32 flags);
u8 *__fastcall VBase_Deleter_7018f8(u8 *object, void *, u32 flags);

// 0x5d0a50 (stdcall; position, unused, first, last, pops 16): the nodes of a doubly linked ring ({next, prev}) from first up to (not
// including) last are moved to before position (a splice); the links are rewritten in the original's order. Returns first (EAX).
u32 __stdcall Ring_Splice(u8 *position, u32 unused, u8 *first, u8 *last);

}  // extern "C"
