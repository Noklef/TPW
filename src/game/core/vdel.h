// The scalar deleting destructors that call the destructor of one of the classes of core/vdtor, core/vobj2 and core/small14 and free the object when asked, and the
// constructor and release of the class with the vtable pointer 0x702a90: reimplementation of 0x5b55e0, 0x5b58a0, 0x5bced0, 0x5bd310, 0x5bd720, 0x5e4000, 0x4f1260,
// 0x5ccd80, 0x5d8b00, 0x5d8b40 and 0x5d8b20.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5b55e0 / 0x5b58a0 / 0x5bced0 / 0x5bd310 / 0x5bd720 / 0x5e4000 / 0x4f1260 / 0x5ccd80 (thiscall; flags, pops 4): Class702880_Dtor / Class702890_Dtor /
// Class7028d0_Dtor / Class7028f8_Dtor / Class702920_Dtor / Class702da0_Dtor / Class7099a4_Dtor / Counted2_Clear on the object; when bit 0 of the flags is set the object is
// freed too. Returns the object.
u8 *__fastcall Class702880_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702890_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class7028d0_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class7028f8_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702920_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class702da0_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Class7099a4_ScalarDtor(u8 *object, void *, u32 flags);
u8 *__fastcall Counted2_ScalarDtor(u8 *object, void *, u32 flags);

// 0x5d8b00 (thiscall): Class7029d0_Ctor on the object, then the vtable pointer 0x702a90 and +0x20 cleared. Returns the object.
// 0x5d8b40 (thiscall): the vtable pointer 0x702a90 is set and Class7029d0_Release run (returns what it returns).
// 0x5d8b20 (thiscall; flags, pops 4): the release, and the object freed when bit 0 of the flags is set. Returns the object.
u8 *__fastcall Class702a90_Ctor(u8 *object);
u32 __fastcall Class702a90_Release(u8 *object);
u8 *__fastcall Class702a90_ScalarDtor(u8 *object, void *, u32 flags);

}  // extern "C"
