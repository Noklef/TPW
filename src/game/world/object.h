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
}
