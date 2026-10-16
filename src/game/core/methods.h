// Small methods, many of which report a result through a pointer to a status (0 for done, -1 for refused): reimplementation
// of 0x6d4b00, 0x6d4b10, 0x6c5ce0, 0x6c6f90, 0x6b00a0, 0x6bbe40, 0x6d6c70, 0x6d6cb0, 0x6d6db0, 0x6af6f0, 0x6af060,
// 0x6adf20 and 0x6ace60. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6d4b00 (thiscall): the dword at +0x18 of what the dword at +0x14 points to; 0 when that is 0.
u32 __fastcall Obj_Get18Of14(const u8 *object);

// 0x6d4b10 (thiscall; out, node, unused, pops 12): when the dword at +0x14 is not 0 the byte at +0x1b of *node gets bit 6
// set, its dword at +4 is set to 1 and *out = 0; else *out = -1. Returns out.
u32 *__fastcall Obj_MarkNodeIf14(u8 *object, void *, u32 *out, u8 **node, u32 unused);

// 0x6c5ce0 (thiscall; out, on, pops 8): when bit 0 of the dword at +0x28 is set, its bit 1 is set (on) or cleared (not on)
// and *out = 0; else *out = -1. Returns out.
u32 *__fastcall Obj_SetBit1At28(u8 *object, void *, u32 *out, u32 on);

// 0x6c6f90 (thiscall; rate, pops 4): the bits 2, 3 and 4 of the byte at +0x20 are replaced by 8 for a rate (low 16 bits)
// of 22050, 0x10 for 44100 and 4 for any other. Undefined return.
void __fastcall Obj_SetRateBits20(u8 *object, void *, u32 rate);

// 0x6b00a0 (thiscall; word, pops 4): when the byte at +0x10 is 0 and the cursor at +4 has reached the end at +8 it is set
// to 1; when it is still 0 the word is written at the cursor, which moves on by 2. Undefined return.
void __fastcall Obj_PutWord(u8 *object, void *, u32 word);

// 0x6bbe40 (thiscall; unused, key, flag, pops 12): when the key is the dword at +0x2c, bit 0 of the dword at +0x30 is
// cleared, +0x24 = 0 and, unless the flag is set or bits 6, 11 or 14 are, bit 4 is set. Returns the key when it is not
// that, else the dword at +0x30.
u32 __fastcall Obj_ClearBit0If2c(u8 *object, void *, u32 unused, u32 key, u32 flag);

// 0x6d6c70 (thiscall; out, source, pops 8): the dword at +4 = the source's dword at +0x44 when bit 8 of its dword at
// +0x4c is set, else the one at +0x40; *out = 0. Returns out.
u32 *__fastcall Obj_Pick40Or44(u8 *object, void *, u32 *out, const u8 *source);

// 0x6d6cb0 (thiscall; out, low, pops 8): the rate 11025 (low non-zero) or 22050; when the dword at +0xc already is that
// *out = -1, else it is set and *out = 0. Returns out.
u32 *__fastcall Obj_SetRateAtC(u8 *object, void *, u32 *out, u32 low);

// 0x6d6db0 (stdcall; object, pops 4): the dword at +0x20, or 0x2400 when it is 0.
u32 __stdcall Obj_Get20Or2400(const u8 *object);

// 0x6af6f0 (cdecl; first, last, dest): the 16-byte blocks from first up to last copied to dest; returns the end of what
// was written (dest when there was nothing; the original leaves its caller's value then).
u8 *__cdecl Blocks16_Copy(const u8 *first, const u8 *last, u8 *dest);

// 0x6af060 (thiscall; dest, first, pops 8): the 8-byte blocks from first up to the end pointer at +8 are copied down to
// dest, which becomes the new end. Returns dest.
u8 *__fastcall Vector8_EraseTo(u8 *vector, void *, u8 *dest, const u8 *first);

// 0x6adf20 (thiscall; out, kind, pops 8): a kind of 2, 4 or 5 is stored at +0xa0 and *out = 0; any other gives -1.
// Returns out.
u32 *__fastcall Obj_SetKindA0(u8 *object, void *, u32 *out, s32 kind);

// 0x6ace60 (thiscall; index, pops 4): for an index below the count at +8, the record the table at +0xc names there when
// its low byte is 1 and the dword after its data (rounded up to dwords) is 0: the address of that data (record + 4); else 0.
u8 *__fastcall Records_DataIfTerminated(const u8 *object, void *, u32 index);

}  // extern "C"
