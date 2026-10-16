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

// The destructors of the classes the constructors Class702880_Ctor, Class702890_Ctor (small9) and Class7028b8_Ctor, Class7028e0_Ctor, Class702908_Ctor (small10)
// build (0x5b5600 / 0x5b56c0 / 0x5bcef0 / 0x5bd330 / 0x5bd740 for the vtable pointers 0x702880 / 0x702890 / 0x7028d0 / 0x7028f8 / 0x702920; thiscall): the vtable
// pointer is set; the shared array record at +0x30 loses a use and at none is destroyed (its array of 0x30-byte elements by CRT_VectorDtorIterator with the
// function the dword at 0x6fd500 holds, the array's block and the record freed) and the pointer cleared; the member list at +0x2c is emptied (each object popped from
// its head is deleted through its slot 0 with 1); +0x24 and +0x28 are cleared and the list itself deleted (slot 0 with 1). The deleting versions (0x5b98a0 / 0x5b9970 /
// 0x5be810 / 0x5be8e0 for 0x702880 / 0x702890 / 0x7028d0 / 0x7028f8; flags, pops 4: bit 0 also frees the object) return the object.
void __fastcall Class702880_Dtor(u8 *object);
void __fastcall Class702890_Dtor(u8 *object);
void __fastcall Class7028d0_Dtor(u8 *object);
void __fastcall Class7028f8_Dtor(u8 *object);
void __fastcall Class702920_Dtor(u8 *object);
u8 *__fastcall Class702880_DeletingDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702890_DeletingDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class7028d0_DeletingDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class7028f8_DeletingDtor(u8 *object, void *, u32 flags);

}  // extern "C"
