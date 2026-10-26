// The live objects list: reimplementation of 0x5775d0 and 0x577600, and of constructors (0x579cd0, 0x5407a0) and a deleting destructor
// (0x57ca00) of classes that register with it.
//
// Classes sharing the abstract virtual base with vtable 0x6fdce8 (the fonts, 0x48f1ff; the
// decoded textures, 0x56b168; some engine parts) add that base to a global list of 0x200 when
// built and take it out when destroyed. The list (0x87a6d8, count 0x87aed8) is only ever written:
// nothing in the exe walks it.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5775d0 (cdecl; object): appended when there is room (the rest are dropped); 1 either way.
u32 __cdecl LiveObjects_Add(void *object);

// 0x577600 (cdecl; object): the last match, searching from the end, is replaced by the last entry
// and the count drops; 1, or 0 when it is not there.
u32 __cdecl LiveObjects_Remove(void *object);

// 0x579cd0 (thiscall; most derived: pops 4; name unconfirmed): when most derived, +0 = the virtual base table 0x701a08 (the base at
// +0x154) and the base's vtable +0x154 = 0x6fdce8. Two identity matrices at +0x10 and +0x50; the base's vtable (at the offset the
// table gives) = 0x7019f8 and the dword before it = that offset - 0x154; +0x90 = +0x94 = -1; +4, +8, +0x11c, +0x124-+0x140 and the
// 0x21 dwords from +0x98 = 0; then the base (this plus the offset; 0 for no object) added to the live objects. Returns this.
u8 *__fastcall LiveObject7019f8_Construct(u8 *object, void *, u32 most_derived);

// 0x5407a0 (thiscall; most derived: pops 4; the static initialiser 0x540860 builds 0x876420 with it; name unconfirmed): the class
// PtrArray_Init and RendererPart_Ctor build inline (the sprite set). When most derived, +0 = the virtual base table 0x6ff318 (the base
// at +0x20) and the base's vtable +0x20 = 0x6fdce8. The base's vtable (at the offset the table gives) = 0x6ff308 and the dword before
// it = that offset - 0x20 (read again); +4, +8, +0x10 = 0, +0xc = -1, +0x14, +0x18 = 0; then the base (this plus the offset, read
// again; 0 for no object) added to the live objects. Returns this.
u8 *__fastcall LiveObject6ff308_Construct(u8 *object, void *, u32 most_derived);

// 0x57ca00 (thiscall on the virtual base, 0xa4 into the object; flags: pops 4; name unconfirmed): the base's vtable (at the offset
// the object's table gives) = 0x701b30 and the dword before it = the offset - 0xa4 (the offset read again after each store), the
// base (the object plus the offset, read again; 0 when the object is) taken out of the live objects, +0xa4 = 0x6fdce8, and with flag
// 1 the object freed (0x67acb0). Returns the object.
u8 *__fastcall LiveObject701b30_DeletingDtor(u8 *base, void *, u32 flags);

// 0x57abf0 (thiscall on the virtual base, 0x154 into the object; flags: pops 4; name unconfirmed): as 0x57ca00 for the class
// LiveObject7019f8_Construct builds: the vtable 0x7019f8 and the offset - 0x154 stored, the base taken out of the live objects, then
// its model unloaded (ObjModel_Unload, model/objmodel), +0x154 = 0x6fdce8 and with flag 1 the object freed. Returns the object.
u8 *__fastcall LiveObject7019f8_DeletingDtor(u8 *base, void *, u32 flags);
}
