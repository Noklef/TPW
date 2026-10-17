// The objects (thing model 3: rides, shops, facilities; serialized at 0x4db7d0): reimplementation
// of 0x4dd4e0, 0x4e1670, 0x4e1690, 0x4e1e00, 0x4e2190, 0x4e21c0, 0x4e2710 and 0x4e2b50 (thiscall).
// +0xe the definition's id; +0x18c mQualityOfGoods, +0x190 mChanceOfWinning; +0x1a0 mNumCustomers,
// +0x1a4 mNumWalkAways, and their histories, CircBufU32s of 30 at +0x1a8 and +0x230 (mTemp first,
// the count so far); +0x340 a running average of the customers' happiness changes.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4dd4e0: the object's definition, ObjectDefs_Find on gObjectDefs (0x7890a0) by its id (+0xe).
u8 *__fastcall Object_GetDef(const u8 *object);

// 0x4e1690 / 0x4e1670: a customer / a walk-away counted, in all and in its history's mTemp.
void __fastcall Object_AddCustomer(u8 *object);
void __fastcall Object_AddWalkAway(u8 *object);

// 0x4e1e00 (change): +0x340 = change when it is 0, else (+0x340 + change) / 2 (signed, toward 0).
void __fastcall Object_AddHappinessChange(u8 *object, void *, s32 change);

// 0x4e2190 / 0x4e21c0 (value): mQualityOfGoods / mChanceOfWinning = the value's low byte.
void __fastcall Object_SetQualityOfGoods(u8 *object, void *, u32 value);
void __fastcall Object_SetChanceOfWinning(u8 *object, void *, u32 value);

// 0x4e2710 (&out): the definition's u16 +0x284 to *out. Returns out.
u16 *__fastcall Object_DefWord284(const u8 *object, void *, u16 *out);

// 0x4e2b50: whether the definition's +0x9c is 3.
u32 __fastcall Object_DefIsKind3(const u8 *object);

// 0x535600 (thiscall; kind, pops 4): whether the kind is allowed beside the kind in the dword at +8 (see the code: 0 always, 4 only
// against anything but 4, 0x15 and 1 with their partners, equal kinds, 0 as the object's kind, 4 against 1, and 3 against 1 only while the
// dword at 0x820ab8 is 0).
u32 __fastcall Kinds_Compatible(const u8 *object, void *, u32 kind);

// 0x4d27a0 (thiscall; id, amount, kind, extra, pops 16): the selected slot (the index at +0x3cc, within 0..19, of the 48-byte slots from +0xc)
// is checked: when the byte at +0x3d9 is not 0, the slot's dword at +0 equals id, its word at +0xc equals the low 16 bits of kind, and the word at
// +0x3e0 is 0 or equal to the low 16 bits of extra, the dword at +0x3dc gains amount. Undefined return.
void __fastcall Slots_AddIfMatch(u8 *object, void *, u32 id, u32 amount, u32 kind, u32 extra);

// 0x503a20 (thiscall; slot, pops 4): the entries (20 bytes from +0xc) are chosen by the dwords at +0x1414 + 16 * slot (0 when below 0): for slot 4
// the dword at +0x1ac of the object definition (word +0xc of the entry; ObjectDefs_Find) at 0x40 * (the byte at +0x15) + 0x1b8; for the others
// the dword at +4 of the object control record (the entry's word at +0xc; the game's table, ObjectControls_FindUncached quiet).
u32 __fastcall Slots_GetValue(const u8 *object, void *, u32 slot);

// 0x5b6230 (thiscall): 1 when any of the dwords at +0 and +0x6c, the dword at +0xc of each of the nine blocks pointed to from +0xe0..+0x100, or the
// dwords at +0x324 and +0x38c is not 0 (in that order), otherwise 0.
u32 __fastcall Obj_HasActivity(const u8 *object);

// 0x4e0220 (thiscall; out, pops 4): the word through out = the handle at +0x5e (0 for none). While the game's clock (the dword at +0x1da70c of the object
// [0x80239c]) is past the stamp at +0x60 plus 100 the handle is kept only if the thing it names (the table at 0x7cfb90) passes Person_HasWord (world/person) with
// this object's own handle (the word at +0); otherwise +0x5e and +0x60 are cleared and 0 is stored. Returns out.
u16 *__fastcall Obj_GetTrackedHandle(u8 *object, void *, u16 *out);

// 0x4e0370: 1 when the dword at 0x19c is 0x1, else 0.
u32 __fastcall Is19c_Eq1_4e0370(const u8 *o);

// 0x4e0380: 1 when the dword at 0x19c is 0x4, else 0.
u32 __fastcall Is19c_Eq4_4e0380(const u8 *o);

// 0x4e03e0: 1 when the dword at 0x19c is 0x2, else 0.
u32 __fastcall Is19c_Eq2_4e03e0(const u8 *o);

// 0x4e0440: 1 when the dword at 0x68 is 0, else 0.
u32 __fastcall Is68_Zero_4e0440(const u8 *o);

// 0x4dd7f0 (thiscall on the object; capacity, pops 4; the debug print is left out): the low byte of the capacity is held to the object definition's limits (the dwords at +0x124
// and +0x128 when their sum is above 0; the minimum first), set as element 2 of the object's ride record (Rides_SetElement of Rides_FindById of the dword at +0x24) and kept at +0x5d.
void __fastcall Object_SetCapacity(u8 *object, void *, u32 capacity);

// 0x52c2b0 (cdecl; the object): 1 unless the object definition's track type (+0x9c) is 1 or 2 and the cell a few steps ahead (by the angle at +0x10: 0 is (+4, +1), 0x5a (+1, -5), 0xb4
// (-5, -2), 0x10e (-2, +4) from the cell of columns/rows at +5 and +7; any other angle stays) and the one beside it (the second offsets: 0 is (-2, +1), 0x5a (+1, +1), 0xb4 (+1, -2), 0x10e (-2, -2)) each have exactly one neighbour
// bit (MapCell_NeighbourCount, on the 0x28-byte cells of the world at [0x7cf83c]).
u32 __cdecl Object_TrackNeighboursOk(const u8 *object);

// 0x4e02f0 (thiscall): 0 when Obj_GetTrackedHandle gives 0; else Person_HasWordInState (world/person) of the thing the handle at +0x5e names (the table
// at 0x7cfb90) with this object's own handle (the word at +0).
u32 __fastcall Obj_TrackedPersonInState(u8 *object);

// 0x4dad20 (thiscall, this unused; two stack arguments, unused, pops 8): for each thing of the list from the u16 at +0x1da746 of the object [0x80239c]
// (the table at 0x7cfb90; the next at +0xc) that is in state 1 or 2 (+0x19c) or has the dword at +0x64 set: when Obj_GetTrackedHandle gives a handle,
// Fn_4e0210 (core/initializers: +0x5e and +0x60 = 0) lets it go.
void __fastcall Objects_ReleaseTracked(void *self, void *, u32 unused1, u32 unused2);
}
