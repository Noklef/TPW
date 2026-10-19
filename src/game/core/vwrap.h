// Small functions that call a virtual method of an object and little else: reimplementation of 0x5c5510, 0x617dd0, 0x5d60d0, 0x5d60f0, 0x46c330,
// 0x498b90, 0x4987d0, 0x578390, 0x5c5530, 0x5c5560 and 0x51af90. The slots are named by their vtable offsets until the classes are known.
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

// 0x5c5000 (cdecl; object; name unconfirmed): a jump to the object's slot 0x14 (no arguments: the object stays on the stack for
// the caller to pop). Returns what it returns.
u32 __cdecl Object_CallSlot14(u8 *object);

// 0x5c4f60 (cdecl; a, b, c, object): the object's slot 8 called with a and c * b.
u32 __cdecl Object_CallSlot8Product(u32 a, u32 b, u32 c, u8 *object);

// 0x565560 (thiscall; a, pops 4): 1 when the dwords at +0x20 or +0x24 of a are not above 0 (signed), otherwise the object's slot 0x10 called with a.
u32 __fastcall Obj_CallSlot10IfSized(u8 *object, void *, u8 *a);

// 0x565930 (thiscall; a, pops 4): when the dword at +0xc of the object and the dword at +4 of a share a bit the object's slot 0x10 is called with a: 0 if it
// returned 0, otherwise the bit 4 of a's first dword is set; then 1.
u32 __fastcall Obj_ApplyIfShared(u8 *object, void *, u8 *a);

// 0x48ccd0 (thiscall; a, pops 4): the object's slot 0x2c when a is the dword at 0x7c2658, otherwise a.
u32 __fastcall Obj_CallSlot2cIfGlobal(u8 *object, void *, u32 a);

// 0x4b7a00 (thiscall; a, pops 4; name unconfirmed): 0 when the dword at +0x10 is 0, else Obj_CallSlot2cIfGlobal(object, a).
u32 __fastcall Obj_CallSlot2cIfGlobalIf10(u8 *object, void *, u32 a);

// 0x569ef0 / 0x5667a0 (cdecl): when the dword at 0x879acc / 0x879ac0 is not 0 the object it points to is deleted (slot 0 with 1) and the dword cleared. Returns 1
// when there was one, otherwise 0.
u32 __cdecl Global879acc_Delete();
u32 __cdecl Global879ac0_Delete();

// 0x569e10 / 0x5666f0 (cdecl; names unconfirmed): their creators. When the dword is 0: a new object (the game's heap) of 0x38 bytes, its virtual base
// (Class701260_Ctor at +8) given the vtable 0x7013c8 and +0xc of it 0x1000 / of 0x5c bytes (Class701378_Ctor, most derived); the dword = the virtual base
// part, whose slot 1 is called, 1. 0 when there was one, or when the allocation failed (the dword 0, the error (3, 0xb0754000) pushed).
// (The original's exception frame only frees the block when a constructor throws, which these do not.)
u32 __cdecl Global879acc_Create();
u32 __cdecl Global879ac0_Create();

// 0x47e990 (cdecl): when the dword at 0x7b4b84 is not 0 the object it points to is deleted (slot 0 with 1); the dword is left.
void __cdecl Global7b4b84_Delete();

// 0x4b3f20 (cdecl): the objects the dwords at 0x7cc2fc, 0x7cc300 and 0x7cc304 point to (when not 0) are each released (slot 8 with 1); the dwords are left.
void __cdecl Globals7cc2fc_Release();

// 0x48eaf0 (cdecl): over the ten 0x208-byte records from 0x7c26b0: those whose word at +4 is 0 and whose first dword is set have that object released (slot 8 with
// 1) and the dword cleared.
void __cdecl Table7c26b0_ReleaseIdle();

// 0x57c360 (cdecl; name unconfirmed): without an object at [0x8bba48], a new one of 0xa8 bytes (the game's heap) there: vbtable 0x701b40 at +0,
// its virtual base (at the vbtable's +4 offset, +0xa4) given vtable 0x701b30 over the 0x6fdce8 first written there and its displacement
// (the dword before it, offset - 0xa4), +4 and +8 = -1, the function pointers 0x51f280 / 0x51faa0 / 0x51fd20 / 0x51f370 / 0x51ef30 /
// 0x51f2f0 / 0x51f240 at +0x84 to +0x9c, and the virtual base added to the live objects (LiveObjects_Add). 1; 0 when there was one, or when
// the allocation failed (the error (3, 0xb0754000) pushed onto the engine's list, EngineErrors_Push).
u32 __cdecl Global8bba48_Create();

// 0x57c4a0 (cdecl): when the dword at 0x8bba48 is not 0 the virtual base of the object it points to (the offset in dword +4 of its vtable) is deleted (its slot 0
// with 1) and the dword cleared. Returns 1 when there was one, otherwise 0.
u32 __cdecl Global8bba48_DeleteVBase();

// 0x576230 (thiscall on a list node {object, next}): while the object's slot 8 returns 1 its slot 4 is called and the next node taken.
void __fastcall ListNodes_Drain(u8 *node);

// 0x591a80 (thiscall on the field 0x4c into an object): the count there is lowered; when it is no longer above 0 the virtual base of the object 0x54 below (a pointer
// at its +4; the offset in dword +4 of its vtable, from 4 past the object) gets its slot 0x1c called with 1. Returns the count.
s32 __fastcall Refcounted_Release(u8 *field);

// 0x497b60 (cdecl): when the dword at 0x7ca1bc is not 0 the object it points to is released (slot 8 with 1); the dword is left.
void __cdecl Global7ca1bc_Release();

// 0x51af90 (cdecl; name unconfirmed): when the dword at 0x8023d0 is set, the COM object at 0x8023d8 gets its slot 0x80 called with 0
// when the dword at 0x8023d4 is set too, then is released (IUnknown::Release, slot 8); 0x8023d0 = 0.
void __cdecl Global8023d8_Release();

// 0x48d860 (thiscall; a, pops 4): the slot 0x14 of the object the dword at +0x20 points to, called with a. Returns what it returns.
u32 __fastcall Obj_CallMember20Slot14(u8 *object, void *, u32 a);

// 0x4af9f0 (thiscall; handle, pops 4; name unconfirmed): with the dword at +0x10 set, the object's slot 0x14 called when the thing in the handle's
// slot (its low word; 0x7cfb90) is there and is the one the dword at 0x7c2658 points to. (Its debug check of the thing table left out.)
void __fastcall Obj_CallSlot14IfGlobalThing(u8 *object, void *, u32 handle);

// 0x5e2310 (thiscall; message, key, unused, pops 12; slot 5 of the vtables 0x702cb0, 0x702dd0 and 0x702ec0; name unconfirmed): message 0x10005 with key 0,
// or 0x1000b with key 0xd, calls the object's slot 0x40; 0x1000b with key 0x2500 calls slot 0x3c, with key 0x2700 slot 0x38 (returning what the slot
// returns). Otherwise returns what EAX held: the key (messages 0x10005 and 0x1000b), or the message less 0x1000b.
u32 __fastcall Obj_KeyMessageToSlots(u8 *object, void *, u32 message, u32 key, u32 unused);

// 0x481530 (thiscall on the object at 0x7b51f0, this not used; handle, pops 4; name unconfirmed): Obj_CallSlot14IfGlobalThing on the object at
// 0x7cc218 with the handle.
void __fastcall Obj7b51f0_PassThingTo7cc218(u8 *object, void *, u32 handle);

}  // extern "C"
