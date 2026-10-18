// Sweeps over the things that call a function for each object of a kind: reimplementation of 0x482040, 0x482100, 0x4821b0, 0x482270, 0x482340 and 0x482400 (cdecl). Each
// walks the things newest first (the world at [0x7cf83c]), considers the objects (model byte at +2 is 3) that pass a test and calls the callback (cdecl, the object as its
// argument) for each; returns how many it called it for. The tests are on the definition word at +0x284 (Object_DefWord284), the definition (Object_GetDef) and the
// object's flag byte at +0x32.
#pragma once

#include "../../../re/types.h"

typedef void(__cdecl *ObjectCallback)(u8 *object);

extern "C" {

// 0x482040 (kind, callback): the word is 0 and the dword at +0x4ac of the definition is the kind.
s32 __cdecl Objects_ForEachOfKind(u32 kind, ObjectCallback callback);

// 0x482100 (callback): the word is not 0.
s32 __cdecl Objects_ForEachWordSet(ObjectCallback callback);

// 0x4821b0 (callback): the word is not 0 and the dword at +0x288 of the definition is 2.
s32 __cdecl Objects_ForEachUpgrade2(ObjectCallback callback);

// 0x482270 / 0x482340 / 0x482400 (callback): the word is 0, the definition's +0x4ac is 3 and the flag byte has bit 0 / neither of bits 0 and 1 / bit 1.
s32 __cdecl Objects_ForEachKind3Flag1(ObjectCallback callback);
s32 __cdecl Objects_ForEachKind3NoFlags(ObjectCallback callback);
s32 __cdecl Objects_ForEachKind3Flag2(ObjectCallback callback);

// 0x481bc0 (mask, callback; name unconfirmed): the callback called for the things of every group whose bit is set in the mask, in this
// order, and how many times it was called: 1 every staff thing of the table (Thing_IsStaff), 2 / 4 / 8 / 0x10 / 0x20 the handymen,
// mechanics, guards, entertainers and researchers (the lists World_FirstHandyman and the like start, chained through the thing's word
// at +0x210), 0x40 the things of model byte 1, 0x80 the objects (model byte 3) whose definition word +0x284 is 0 and definition +0x4ac is
// 0; 0x100 / 0x200 / 0x400 the objects of kind 2 / 1 / 3 (Objects_ForEachOfKind), 0x800 Objects_ForEachKind3NoFlags, 0x1000
// Objects_ForEachKind3Flag1, 0x2000 Objects_ForEachWordSet, 0x4000 Objects_ForEachUpgrade2 and 0x8000 Objects_ForEachKind3Flag2.
s32 __cdecl Things_ForEachSelected(u32 mask, ObjectCallback callback);

// 0x48ca00 (thiscall; value, pops 4; name unconfirmed): the count 0x7c2660 zeroed, then raised (Things_CountOne) for each thing of the
// group the owner's +4 picks: -2 the model 1 things (mask 0x40), -1 the staff (1), 0 the kind 0 objects (0x80), 1 / 2 / 3 the kind 1 /
// 2 / 3 objects (0x200 / 0x100 / 0x800 (sic: the kind 3 objects with neither flag)), 0x3ea those with flag 1 (0x1000), 0x3eb with flag
// 2 (0x8000), 0x3ec the upgrade 2 ones (0x4000), any other none (Things_ForEachSelected); then 0x7c2658 = value. Returns the count.
s32 __fastcall Things_CountGroup(const u8 *owner, void *, u32 value);

// 0x48cad0 (cdecl; thing, unused; name unconfirmed): the count 0x7c2660 raised; returns 1.
s32 __cdecl Things_CountOne(u8 *thing);

// 0x48caf0 (thiscall; name unconfirmed): the owner's slot 0x3c called, then the next thing of the group its +4 picks (the masks as
// Things_CountGroup's, as a category) after 0x7c2658, from the start past the last (Things_FindNextOfCategoryWrap; any other group: none);
// when there is one and it is not 0x7c2658 already, 0x7c2658 = it and the owner's slot 4 called.
void __fastcall Things_StepGroup(u8 *owner);

}  // extern "C"
