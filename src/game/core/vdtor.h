// Constructors, destructors and releases that register an object with, or hand it to, a virtual method of another object (a global one, a member or a virtual
// base): reimplementation of 0x5e3b90, 0x5e3bd0, 0x5e3c00, 0x5d5700, 0x5d5720, 0x5d5750, 0x5d9b20, 0x5dc3b0, 0x5dc550, 0x5dd7f0, 0x5ddaa0 and 0x564a60.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5e3b90 / 0x5e3c00 / 0x5e3bd0: vtable pointer 0x702d48; the object [0xf85614] points to is told (slot 0x18 for the constructor, which also clears +4; 0x1c for the
// others) about the object. The constructor returns the object, the release what the call returned, the deleting destructor (flags, pops 4: bit 0 also frees the
// object) the object.
u8 *__fastcall Class702d48_Ctor(u8 *object);
u32 __fastcall Class702d48_Release(u8 *object);
u8 *__fastcall Class702d48_DeletingDtor(u8 *object, void *, u32 flags);

// 0x5d5700 / 0x5d5750 / 0x5d5720: the same for vtable pointer 0x7029d0 and the object [0xf82884] points to (slot 0 for the constructor, slot 4 for the others).
u8 *__fastcall Class7029d0_Ctor(u8 *object);
u32 __fastcall Class7029d0_Release(u8 *object);
u8 *__fastcall Class7029d0_DeletingDtor(u8 *object, void *, u32 flags);

// 0x5d9b20 / 0x5dc3b0 / 0x5dc550 (flags, pops 4): vtable pointer 0x702ae8 / 0x702bb4 / 0x702bb8; the member object at the pointer +4 / +8 / +8 is called (slot
// 8 / 0x3c / 0x38) with the object; bit 0 of flags also frees it. Returns the object.
u8 *__fastcall Class702ae8_DeletingDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702bb4_DeletingDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702bb8_DeletingDtor(u8 *object, void *, u32 flags);

// 0x5dd7f0 / 0x5ddaa0 (thiscall): the object's slot 0x20; then of the object that the dword at +4 of the object [0xf85614] points to slot 0x1c (and for 0x5ddaa0
// then slot 0x48, which the first returns the result of by jumping to it). Return what the last call returned.
u32 __fastcall Obj_CallSlot20ThenSub1c(u8 *object);
u32 __fastcall Obj_CallSlot20ThenSub1cAnd48(u8 *object);

// 0x564a60 (thiscall on the object 0x18 into a class; a, pops 8): the virtual base (at the offset in dword +4 of the vtable of the class pointer 0x18 below
// the object, from that pointer) gets its slot 8 called with a. Returns what it returns.
u32 __fastcall VBase_CallSlot8(u8 *object, void *, u32 a, u32 unused);  // (pops 8; only the first is passed on)

// The destructors of the classes the constructors Class702880_Ctor, Class702890_Ctor (core/vdtor) and Class7028b8_Ctor, Class7028e0_Ctor, Class702908_Ctor (core/vdtor)
// build (0x5b5600 / 0x5b56c0 / 0x5bcef0 / 0x5bd330 / 0x5bd740 for the vtable pointers 0x702880 / 0x702890 / 0x7028d0 / 0x7028f8 / 0x702920; thiscall): the vtable
// pointer is set; the shared array record at +0x30 loses a use and at none is destroyed (its array by CRT_VectorDtorIterator with a WEACITYR destructor:
// 0x30-byte ccCityInfo (0x6fd500), 0x66-byte ccParkInfo (0x6fd504), 0x10-byte ccThemeInfo (0x6fd50c), 0x26-byte ccServerInfo (0x6fd508) and 0x34-byte
// records (0x6fd510) in that order of the classes; the array's block and the record freed) and the pointer cleared; the member list at +0x2c is emptied (each object popped from
// its head is deleted through its slot 0 with 1); +0x24 and +0x28 are cleared and the list itself deleted (slot 0 with 1). The deleting versions (0x5b98a0 / 0x5b9970 /
// 0x5be810 / 0x5be8e0 / 0x5be9b0 for 0x702880 / 0x702890 / 0x7028d0 / 0x7028f8 / 0x702920; flags, pops 4: bit 0 also frees the object) return the
// object.
void __fastcall Class702880_Dtor(u8 *object);
void __fastcall Class702890_Dtor(u8 *object);
void __fastcall Class7028d0_Dtor(u8 *object);
void __fastcall Class7028f8_Dtor(u8 *object);
void __fastcall Class702920_Dtor(u8 *object);
u8 *__fastcall Class702880_DeletingDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702890_DeletingDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class7028d0_DeletingDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class7028f8_DeletingDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702920_DeletingDtor(u8 *object, void *, u32 flags);

// 0x5b57a0 (thiscall; the class built at 0x5b536f, derived from 0x702890's): the vtable pointer 0x702828, the wide string at +0x34
// destroyed (WString_Dtor, core/string), then Class702890_Dtor's steps.
void __fastcall Class702828_Dtor(u8 *object);

// 0x5dc580 (thiscall): the 0x702d48 base constructor (Class702d48_Ctor), the vtable 0x702bc8, a 0x10-byte object {vtable 0x702bc0, three zero dwords} at +8 and another {vtable
// 0x702bbc, three zero dwords} at +0x6c (each 0 when the allocation fails), +0x78 and +0x74 cleared and the floats 0.4, 0.15, 0.5, 300 at +0x1c, +0x20, +0x24, +0x38 and 0.2,
// 0.075, 0.25, 150, 1.5 at +0x28, +0x2c, +0x30, +0x3c, +0x40. Returns the object.
u8 *__fastcall Class702bc8_Ctor(u8 *object);

// 0x5bce60 / 0x5bd2a0 / 0x5bd6b0 (thiscall): like Class702880_Ctor (see core/vdtor) but the vtable pointer at +0 is 0x7028d0 / 0x7028f8 / 0x702920
// while the 16-byte member (vtable 0x7028c8 / 0x7028f0 / 0x702918) is made and 0x7028b8 / 0x7028e0 / 0x702908 after it.
u8 *__fastcall Class7028b8_Ctor(u8 *object);

u8 *__fastcall Class7028e0_Ctor(u8 *object);

u8 *__fastcall Class702908_Ctor(u8 *object);

// 0x5bdc10 / 0x5be320 / 0x5be750 (thiscall): the vtable pointer 0x702938 / 0x702940 / 0x702948 is set; the object is taken off the list the object at +4 has at +0x2c (nodes
// {object, next, previous}; head +4, tail +8, count +0xc; the node is freed); then the strings at +0x44, +0x20 and +0xc / +0x34, +0x20 and +0xc / +0x4c, +0x20 and +0xc are destroyed (WString_Dtor / String_Dtor / WString_Dtor).
void __fastcall Class702938_Dtor(u8 *object);

void __fastcall Class702940_Dtor(u8 *object);

void __fastcall Class702948_Dtor(u8 *object);

// 0x5be0e0 / 0x5be530: the same for the classes with the vtable pointers 0x70293c (the parent list only when the parent at +4 is set; wide strings at +0x94, +0x34, +0x20, +0xc and a
// narrow one at +0x7c) and 0x702944 (wide strings at +0x6c, +0x54, +0x40, +0x24, +0x10).
void __fastcall Class70293c_Dtor(u8 *object);

void __fastcall Class702944_Dtor(u8 *object);

// (thiscall): the vtable pointer 0x703780 is set, the object at +8 (when there is one) gets its slot 0 called (no argument), and the vtable pointer 0x703770 is set.
void __fastcall Class703780_DtorA(u8 *object);

void __fastcall Class703780_DtorB(u8 *object);

void __fastcall Class703780_DtorC(u8 *object);

u8 *__fastcall Class702b08_Ctor(u8 *object);

u8 *__fastcall Class702d78_Ctor(u8 *object);

void __fastcall Class702b08_Dtor(u8 *object);

void __fastcall Class702cb0_Dtor(u8 *object);

void __fastcall Class702d78_Dtor(u8 *object);

// Each sets the vtable pointer named and runs Class702cb0_Dtor on the object.
void __fastcall Class702ec0_Dtor(u8 *object);

void __fastcall Class702dd0_Dtor(u8 *object);

// 0x5e3fc0 / 0x5e3ff0 (thiscall destructors): set the vtable pointer named, then Class702bc8_Dtor on the object (which sets its own).
void __fastcall Class702e70_Dtor(u8 *object);
void __fastcall Class702e20_Dtor(u8 *object);

// 0x617f40 (thiscall destructor): sets the vtable pointer 0x7037b8, frees the block at +8 of every node of the list at +8 (head, size at +0xc), then every node and the
// head, and clears the pointer and the size.
void __fastcall Class7037b8_Dtor(u8 *object);

// 0x5c8d80 (thiscall): Class70295c_Ctor, then the dwords at +0x18, +0x1c and +0x420 = 0, the vtable pointer at +0 = 0x702958 and the 256
// dwords from +0x20 = 0. Returns the object.
u8 *__fastcall Class702958_Ctor(u8 *object);

// 0x5b96e0 / 0x5b9740 (thiscall): the fields at +4..+0x30 cleared, the vtable pointer at +0 = 0x702880 / 0x702890, a new member of 16 bytes
// (vtable 0x7028a8 / 0x7028ac, +4, +8, +0xc = 0) at +0x2c (0 for none). Returns the object.
u8 *__fastcall Class702880_Ctor(u8 *object);

u8 *__fastcall Class702890_Ctor(u8 *object);

// 0x407c30: a linked list destructor: sets the vtable pointer 0x6fd708 and frees every node.
void __fastcall ListDtor6fd708_407c30(u8 *o);

// 0x41c9a0: a linked list destructor: sets the vtable pointer 0x6fd9e4 and frees every node.
void __fastcall ListDtor6fd9e4_41c9a0(u8 *o);

// 0x41eb50: a linked list destructor: sets the vtable pointer 0x6fdb08 and frees every node.
void __fastcall ListDtor6fdb08_41eb50(u8 *o);

// 0x421640: a linked list destructor: sets the vtable pointer 0x6fdb10 and frees every node.
void __fastcall ListDtor6fdb10_421640(u8 *o);

// 0x5193d0: a linked list destructor: sets the vtable pointer 0x700aa0 and frees every node.
void __fastcall ListDtor700aa0_5193d0(u8 *o);

// 0x5ac550: a linked list destructor: sets the vtable pointer 0x7027fc and frees every node.
void __fastcall ListDtor7027fc_5ac550(u8 *o);

// 0x5ac590: a linked list destructor: sets the vtable pointer 0x7027fc and frees every node.
void __fastcall ListDtor7027fc_5ac590(u8 *o);

// 0x5b58c0: a linked list destructor: sets the vtable pointer 0x7028a0 and frees every node.
void __fastcall ListDtor7028a0_5b58c0(u8 *o);

// 0x5b9a80: a linked list destructor: sets the vtable pointer 0x7028b0 and frees every node.
void __fastcall ListDtor7028b0_5b9a80(u8 *o);

// 0x5b9ac0: a linked list destructor: sets the vtable pointer 0x7028b4 and frees every node.
void __fastcall ListDtor7028b4_5b9ac0(u8 *o);

// 0x5beae0: a linked list destructor: sets the vtable pointer 0x70294c and frees every node.
void __fastcall ListDtor70294c_5beae0(u8 *o);

// 0x5beb20: a linked list destructor: sets the vtable pointer 0x702950 and frees every node.
void __fastcall ListDtor702950_5beb20(u8 *o);

// 0x5beb60: a linked list destructor: sets the vtable pointer 0x702954 and frees every node.
void __fastcall ListDtor702954_5beb60(u8 *o);

// 0x5d5800: a linked list destructor: sets the vtable pointer 0x7029f8 and frees every node.
void __fastcall ListDtor7029f8_5d5800(u8 *o);

// 0x5d8610: a linked list destructor: sets the vtable pointer 0x702a88 and frees every node.
void __fastcall ListDtor702a88_5d8610(u8 *o);

// 0x5d8650: a linked list destructor: sets the vtable pointer 0x702a8c and frees every node.
void __fastcall ListDtor702a8c_5d8650(u8 *o);

// 0x5d9f10: a linked list destructor: sets the vtable pointer 0x702afc and frees every node.
void __fastcall ListDtor702afc_5d9f10(u8 *o);

// 0x5da2b0: a linked list destructor: sets the vtable pointer 0x702b28 and frees every node.
void __fastcall ListDtor702b28_5da2b0(u8 *o);

// 0x5dc6c0: a linked list destructor: sets the vtable pointer 0x702c0c and frees every node.
void __fastcall ListDtor702c0c_5dc6c0(u8 *o);

// 0x5dc700: a linked list destructor: sets the vtable pointer 0x702c10 and frees every node.
void __fastcall ListDtor702c10_5dc700(u8 *o);

// 0x5e0370: a linked list destructor: sets the vtable pointer 0x702d00 and frees every node.
void __fastcall ListDtor702d00_5e0370(u8 *o);

// 0x5e3cd0: a linked list destructor: sets the vtable pointer 0x702d98 and frees every node.
void __fastcall ListDtor702d98_5e3cd0(u8 *o);

// 0x5e40a0: a linked list destructor: sets the vtable pointer 0x702f10 and frees every node.
void __fastcall ListDtor702f10_5e40a0(u8 *o);

// 0x5f3900: a linked list destructor: sets the vtable pointer 0x703084 and frees every node.
void __fastcall ListDtor703084_5f3900(u8 *o);

// Three front-end destructors that drain member lists (count +0xc, head +4, tail +8; nodes {item, next, prev}): front first, each node freed and its item
// (when there is one) deleted through its slot 0 with flags 1, then the list deleted the same way and the member cleared. Each has a scalar deleting
// destructor (flags, pops 4: bit 0 also frees the object; returns it).
// 0x5d9f50 / 0x5d9ed0: vtable pointer 0x702af0; the object [0xf83784] points to is told (slot 0x1c); the list at +0x58; the strings at +0x24 and +0x10
// destroyed.
void __fastcall Class702af0_Dtor(u8 *object);
u8 *__fastcall Class702af0_DeletingDtor(u8 *object, void *, u32 flags);
// 0x5dc740 / 0x5dc660: vtable pointer 0x702bc8; the lists at +8 and +0x6c; then the 0x702d48 base's release (Class702d48_Release).
void __fastcall Class702bc8_Dtor(u8 *object);
u8 *__fastcall Class702bc8_DeletingDtor(u8 *object, void *, u32 flags);
// 0x5dfc30 / 0x5dfc10: vtable pointer 0x702ca8; the member at +0xc is told (slot 0x34); the WString at +0x48 and the String at +0x28 destroyed.
void __fastcall Class702ca8_Dtor(u8 *object);
u8 *__fastcall Class702ca8_DeletingDtor(u8 *object, void *, u32 flags);
}  // extern "C"
