// Register shadows written through COM-style calls, a seek check and two list drains: reimplementation of 0x5679b0, 0x567a10, 0x567a70, 0x5c4fa0, 0x5d8a10 and
// 0x5da3e0. The register shadow is a dword at +0x14 of the class (found through the offset in dword +4 of the object's vtable); the member object at +0x24 is called
// COM-style (stdcall, itself the first argument). Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5679b0 (thiscall; value, pops 4): when the low byte of value differs from the shadow's, the member's slot 0x58 is called with (0x13, value >> 4 & 0xf) (an arithmetic
// shift) and then (0x14, value & 0xf), and the shadow's low byte becomes value's.
void __fastcall Shadow_SetByte(u8 *object, void *, s32 value);

// 0x567a10 (thiscall; value, mask, reg, onValue, offValue, pops 20): when mask has a bit that value and the shadow differ in, the member's slot 0x58 is called with (reg, onValue
// when mask & value is not 0, otherwise offValue) and the shadow's bits under the mask become value's.
void __fastcall Shadow_SetBits2(u8 *object, void *, u32 value, u32 mask, u32 reg, u32 onValue, u32 offValue);

// 0x567a70 (thiscall; value, a, mask, b, onValue, offValue, pops 24): the same with slot 0xa0 called with (a, b, the chosen value).
void __fastcall Shadow_SetBits3(u8 *object, void *, u32 value, u32 a, u32 mask, u32 b, u32 onValue, u32 offValue);

// 0x5c4fa0 (cdecl; object, offset, whence): the object's slot 0x10 called with (offset, whence) for whence 0, 1 and 2: whether it returned a negative number; 1 for any
// other whence.
u32 __cdecl Stream_SeekFailed(u8 *object, u32 offset, u32 whence);

// 0x5d8a10 / 0x5da3e0 (thiscall): while the count behind the list pointer at +0xc / +4 is not 0 the head node is taken off (freed), and the object it held (when any) is
// deleted through its slot 0 with 1.
void __fastcall Obj_DrainList0c(u8 *object);
void __fastcall Obj_DrainList04(u8 *object);

}  // extern "C"
