// The counted base class whose vtable is 0x703770 (the sound library's banks and category records derive from it, core/soundcategory; its
// constructor 0x6338b0 is Class703770_Ctor in core/misc): reimplementation of its AddRef and Release, 0x6338c0-0x6338f8 (in the library range
// 0x630ca0-0x658000). The count is at +4; a derived class's vtable +0 is Release, +4 AddRef, +8 its deleting destructor.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6338c0 (thiscall): InterlockedIncrement (+4). Returns its result.
s32 __fastcall Class703770_AddRef(u8 *object);

// 0x6338d0 (thiscall): InterlockedDecrement (+4); at 0 the object is deleted (its vtable +8 (1)) and 0 returned, else the count (+4 read again).
s32 __fastcall Class703770_Release(u8 *object);

// The text library's counted base class (vtable 0x70a078: slot 0 Class703770_Release, 1 Class703770_AddRef, 2 its deleting destructor;
// the resource index (files/resindex) and the glyph renderers derive from it). Names unconfirmed.
// 0x6af760 (thiscall): the vtable 0x70a078, the count (+4) 1. Returns the object.
u8 *__fastcall Class70a078_Ctor(u8 *object);

// 0x6acfd0 (thiscall (flags); slot 2): the vtable 0x70a078, freed (Mem_Free) for flags bit 0. Returns the object.
u8 *__fastcall Class70a078_DeletingDtor(u8 *object, void *, u32 flags);

}  // extern "C"
