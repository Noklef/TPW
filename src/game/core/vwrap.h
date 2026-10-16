// Small functions that call a virtual method of an object and little else: reimplementation of 0x5c5510, 0x617dd0, 0x5d60d0, 0x5d60f0, 0x46c330,
// 0x498b90, 0x4987d0, 0x578390, 0x5c5530 and 0x5c5560. The slots are named by their vtable offsets until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5c5510 (thiscall; a, pops 4): the object's slot 0x18 called with a. Returns what it returns.
u32 __fastcall Obj_CallSlot18(u8 *object, void *, u32 a);

// 0x617dd0 (thiscall; target, pops 4): the target's slot 0x10 called (as the target's method) with the object as its argument. Returns what it returns.
u32 __fastcall Obj_PassToSlot10(u8 *object, void *, u8 *target);

// 0x5d60d0 / 0x5d60f0 (thiscall): the object's slot 0x24 called; the dword at +4 = 1 when it returned 0 / = 0 when it returned something else. Returns what it
// returned.
u32 __fastcall Obj_RaiseIfSlot24Zero(u8 *object);
u32 __fastcall Obj_ClearIfSlot24Set(u8 *object);

// 0x46c330 (stdcall; a, b, pops 8): slot 0x14 of the object the dword at 0x7b05d8 points to, called with a, b. Returns what it returns.
u32 __stdcall Global7b05d8_CallSlot14(u32 a, u32 b);

// 0x498b90 / 0x4987d0 (cdecl): when the dword at 0x7ca1e0 / 0x7ca1c8 is not 0, slot 8 of the object it points to is called with 1 (its deleting destructor).
void __cdecl Global7ca1e0_Release();
void __cdecl Global7ca1c8_Release();

// 0x578390 (cdecl): when the dword at 0x87b1d8 is not 0, slot 0 (the deleting destructor) of the object it points to is called with 1 and the dword is set to 0.
void __cdecl Global87b1d8_Delete();

// 0x5c5530 / 0x5c5560 (thiscall; a, pops 4): slot 0x18 of the sub-object at +0x14 / +0x28 (its own vtable pointer is its first dword) called with a. Returns what it returns.
u32 __fastcall Obj_CallSub14Slot18(u8 *object, void *, u32 a);
u32 __fastcall Obj_CallSub28Slot18(u8 *object, void *, u32 a);

// 0x617a30 / 0x617b00 / 0x617b30 (thiscall; a, pops 4): the member object at +8, when there is one, gets its slot 0x14 / 0x18 / 0x10 called with a (returns what it
// returns); otherwise the dword at +8 of a is cleared and a returned.
u32 __fastcall Obj_ForwardToMember14(u8 *object, void *, u8 *a);
u32 __fastcall Obj_ForwardToMember18(u8 *object, void *, u8 *a);
u32 __fastcall Obj_ForwardToMember10(u8 *object, void *, u8 *a);

// 0x5c4f80 (cdecl; object): the object's slot 4, then its deleting destructor (slot 0 with 1). Returns 0.
u32 __cdecl Object_FinishAndDelete(u8 *object);

// 0x5c4f60 (cdecl; a, b, c, object): the object's slot 8 called with a and c * b.
u32 __cdecl Object_CallSlot8Product(u32 a, u32 b, u32 c, u8 *object);

// 0x565560 (thiscall; a, pops 4): 1 when the dwords at +0x20 or +0x24 of a are not above 0 (signed), otherwise the object's slot 0x10 called with a.
u32 __fastcall Obj_CallSlot10IfSized(u8 *object, void *, u8 *a);

// 0x565930 (thiscall; a, pops 4): when the dword at +0xc of the object and the dword at +4 of a share a bit the object's slot 0x10 is called with a: 0 if it
// returned 0, otherwise the bit 4 of a's first dword is set; then 1.
u32 __fastcall Obj_ApplyIfShared(u8 *object, void *, u8 *a);

// 0x48ccd0 (thiscall; a, pops 4): the object's slot 0x2c when a is the dword at 0x7c2658, otherwise a.
u32 __fastcall Obj_CallSlot2cIfGlobal(u8 *object, void *, u32 a);

}  // extern "C"
