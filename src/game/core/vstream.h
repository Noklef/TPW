// The stream buffer reads (the get area behind two pointers: a pointer to the current position at +0x1c and a pointer to the count of units left at +0x2c; the
// refill is virtual slot 0x10) and two virtual call sequences: reimplementation of 0x4f1eb0, 0x5d0050, 0x4f06c0, 0x5ceab0, 0x4f18d0 and 0x497610.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4f1eb0 / 0x5d0050 (thiscall): the current unit (a word / a byte) when the position pointer is set and the position is below the position plus the count
// (units of 2 / 1 bytes), otherwise what the object's slot 0x10 returns. The word version leaves the high half of the position in the high half of the result
// (sic: the original loads only AX); the byte version is zero extended.
u32 __fastcall WBuf_Peek(u8 *object);
u32 __fastcall Buf_Peek(u8 *object);

// 0x4f06c0 / 0x5ceab0 (thiscall): the object's slot 0x10 is called; when it returned 0xffff / -1 so does this; otherwise the count behind +0x2c is lowered, the position
// behind +0x1c moved on a unit (2 / 1 bytes) and the unit it was at returned (as above for the high half).
u32 __fastcall WBuf_Next(u8 *object);
u32 __fastcall Buf_Next(u8 *object);

// 0x4f18d0 (thiscall; mask, char, pops 8): whether the 16-bit table at +0x10, at the signed byte the object's slot 0x30 returns for (char, 0), has a bit of the low 16 bits
// of mask.
u32 __fastcall CType_Is(u8 *object, void *, u32 mask, u32 character);

// 0x497610 (thiscall): the object's slot 0x34 is called, its result handed to slot 0x24 of the member object at +0x20 (looked up in that object's vtable before the
// first call, called on the member as it is after it); then the object's slots 0x18 and 0x14. Returns what the last returned.
u32 __fastcall Obj_RefreshMember(u8 *object);

// 0x4f1950 / 0x4f1990 (thiscall; mask, first, last, pops 12): the first wide character from first up to last for which the object's slot 8 (is) called with mask and
// the character returns non-zero (AL) / zero; last when there is none.
u16 *__fastcall CType_ScanIs(u8 *object, void *, u32 mask, u16 *first, u16 *last);
u16 *__fastcall CType_ScanNot(u8 *object, void *, u32 mask, u16 *first, u16 *last);

// 0x4f1900 (thiscall; first, last, dest, pops 12): for each wide character from first up to last the object's slot 0x30 is called with it and 0; the signed byte it
// returns picks the word of the table at +0x10 that is stored at the destination. Returns last.
u16 *__fastcall CType_MapWords(u8 *object, void *, u16 *first, u16 *last, u16 *dest);

// 0x4f0e30 / 0x5cf1c0 (thiscall): like WBuf_Peek / Buf_Peek, but when there is no unit the object's slot 0x14 is called and its result handed to slot 8 (the vtable read
// before the first call), whose result is returned.
u32 __fastcall WBuf_PeekConverted(u8 *object);
u32 __fastcall Buf_PeekConverted(u8 *object);

}  // extern "C"
