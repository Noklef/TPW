// Tiny field setters, selectors and list walkers: reimplementation of 0x6c5190, 0x6c51b0, 0x6b7130, 0x6cc760,
// 0x6b9c90, 0x6c6fd0, 0x6b0560, 0x6b05e0, 0x6b6240, 0x6b6970, 0x6b54a0, 0x56afa0, 0x6d48f0, 0x5d0e30 and 0x5d0e50.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c5190 / 0x6c51b0 (thiscall; value, pops 4): the dword at +0x14 gets bit 7 / bit 3 set, and the dword at +0 / +4
// is set to the value, which is returned.
u32 __fastcall Obj_SetFirstFlag80(u8 *object, void *, u32 value);
u32 __fastcall Obj_SetSecondFlag8(u8 *object, void *, u32 value);

// 0x6b7130 (thiscall; byte, pops 4): the byte at +0x44 = the argument's low byte and bit 10 of the dword at +0x30 set;
// returns that dword.
u32 __fastcall Obj_SetByte44Flag400(u8 *object, void *, u32 value);

// 0x6cc760 (thiscall; value, pops 4): the dwords at +0x242c and +0x2430 = value << 3, which is returned.
u32 __fastcall Obj_Set242cTimes8(u8 *object, void *, u32 value);

// 0x6b9c90 (thiscall; a, b, pops 8): +0x54 = a, +0x58 = b and bit 3 of the dword at +0x4c set; returns that dword.
u32 __fastcall Obj_Set54And58Flag8(u8 *object, void *, u32 a, u32 b);

// 0x6c6fd0 (thiscall): a sample rate by the byte at +0x20: 22050 with bit 3 set, else 44100 with bit 4 set, else 11025.
u32 __fastcall Obj_SampleRate20(const u8 *object);

// 0x6b0560 / 0x6b05e0 (thiscall): 0 when the dword at +8 is 0; otherwise the word at +6 / the byte at +5 of what the
// dword at +0xc points to, in the low 16 bits (the high 16 bits are those of that pointer).
u32 __fastcall Obj_WordFrom6(const u8 *object);
u32 __fastcall Obj_ByteFrom5(const u8 *object);

// 0x6b6240 (thiscall): over the chain from the dword at +0x24 (next at +0x34), the nodes whose dword at +0x30 has bit 10
// set have it cleared and bit 8 set.
void __fastcall Chain24_Promote400To100(const u8 *object);

// 0x6b6970 (thiscall; value, pops 4): the first node of the chain from the dword at +0x20 (next at +0x34) whose dword
// at +0x2c equals the value, or 0.
u8 *__fastcall Chain20_Find2c(const u8 *object, void *, u32 value);

// 0x6b6140 (thiscall; value, pops 4): the dword at +0x4c = the value, or 0x1e when that is above it (unsigned). Returns it.
u32 __fastcall Obj_Set4cMin1e(u8 *object, void *, u32 value);

// 0x6b6a50 / 0x6b6a80 (thiscall; node, pops 4): pushes the node on the front of the doubly linked chain whose head is the
// dword at +0x20 / +0x24 (next at +0x34, previous at +0x38). Returns the node.
u8 *__fastcall Chain20_PushFront(u8 *object, void *, u8 *node);
u8 *__fastcall Chain24_PushFront(u8 *object, void *, u8 *node);

// 0x6b9900 (thiscall; status, index, value, pops 12): when the low byte of the index is below 4 the byte at +0x44 + that is
// set to the value's low byte and *status = 0; else *status = -1. Returns status.
u32 *__fastcall Bytes44_Set(u8 *object, void *, u32 *status, u32 index, u32 value);

// 0x6bcb60 (thiscall; value, pops 4): whether bit 1 of the dword at +0x30 is clear and the value is above the dwords at +0x1c
// and +0x3c added (unsigned).
bool __fastcall Obj_PastEnd(const u8 *object, void *, u32 value);

// 0x6c0da0 (thiscall; index, pops 4): the dword at that index of the array at +8, or 0 when there is none or the index is not
// below the count at +4.
u32 __fastcall Array_AtOrZero(const u8 *object, void *, u32 index);

// 0x6c51f0 (thiscall; a, b, pops 8): bit 18 of the dword at +0x14 set, +0x94 = a and +0x98 = b. Returns a.
u32 __fastcall Obj_Set94_98_Flag40000(u8 *object, void *, u32 a, u32 b);

// 0x6b6930 / 0x6b6b00 (thiscall; node, pops 4): takes the node out of the doubly linked chain whose head is the dword at +0x20 /
// +0x24 and pushes it on the front of the other (Chain24_PushFront / Chain20_PushFront). Returns the node.
u8 *__fastcall Chain20_MoveTo24(u8 *object, void *, u8 *node);
u8 *__fastcall Chain24_MoveTo20(u8 *object, void *, u8 *node);

// 0x6b6b40 (thiscall; index, pops 4): the entry the low 16 bits of the index name in a list of pages (the first at the dword at
// +0x1c, the next at +8 of each) of 100 entries of 0x50 bytes from +0xc: the address, or 0 when the list is too short.
u8 *__fastcall Pages100_EntryAt(const u8 *object, void *, u32 index);

// 0x6b6bc0 (thiscall; node, pops 4): the node is inserted into the array of pointers at +0x44 (count at +0x4c), sorted by their
// dword at +0x20 descending: along the array whichever of the node and the entry has the greater key stays, the other carried on
// (an empty slot takes the carried node). Then +0x48 = the last entry's key (0 for none). Does nothing when the node's key is
// not above +0x48. Returns the new +0x48 (the old when nothing was done).
u32 __fastcall Top_Insert(u8 *object, void *, u8 *node);

// 0x6bb950 (thiscall): a constructor: vtable 0x70a7e0 and the fields +4-+0x28, +0x30, +0x3c, +0x40, +0x4c = 0, the bytes +0x45
// and +0x46 = 0xff. Returns the object.
u8 *__fastcall Class70a7e0_Ctor(u8 *object);

// 0x6b85a0 (thiscall; status, flag, pops 8): with a target at +0x24: bit 10 of its dword at +0x28 is set (flag) or cleared and,
// when cleared with a chain owner at +0x20, that owner's chain is promoted (Chain24_Promote400To100); *status = 0. Without a
// target *status = -1. Returns status.
u32 *__fastcall Target24_SetBit400(const u8 *object, void *, u32 *status, u32 flag);

// 0x6b54a0 (thiscall): the low nibble of the byte the dword at +4 points to when the byte at +0 is not 0, else the high.
u32 __fastcall Nibble_Select(const u8 *object);

// 0x56afa0 (thiscall): when the dword at +0x200 is above 0 it is decremented and the address of the 8-byte entry it now
// names is returned; else 0.
u8 *__fastcall Entries8_Pop(u8 *object);

// 0x6d48f0 (thiscall; index, pops 4): for an index up to the dword at +0xc (unsigned), the dword at +8 plus its dword
// at 4 * index plus 8; else 0.
u32 __fastcall Offsets_Resolve(const u8 *object, void *, u32 index);

// 0x5d0e30 (thiscall; a, b, c, pops 12): the targets of the pointers at +0xc, +0x1c and +0x2c are set to a, b and c - b;
// 0x5d0e50 (a, b, pops 8): those of +0x10 and +0x20 to a and of +0x30 to b - a. Both return the last pointer.
u32 *__fastcall Targets3_Set(u8 *object, void *, u32 a, u32 b, u32 c);
u32 *__fastcall Targets2_Set(u8 *object, void *, u32 a, u32 b);

}  // extern "C"
