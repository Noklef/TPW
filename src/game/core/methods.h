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

// 0x6ad900 (thiscall; key, pops 4): looks the low 16 bits of the key up in a chain of range segments at the dword at +8
// (a segment: first +0, next if below +2, last +4, next if above +6, both as byte distances, 0 for none; the values from
// +8, one word each): the word for the key, in the low 16 bits; 0 when it falls in no segment. The high 16 bits are
// those of the segment's address (sic).
u32 __fastcall Ranges_Lookup(const u8 *object, void *, u32 key);

// 0x6ad8f0 (thiscall; key, pops 4): Ranges_Lookup, for a caller that takes it as an adapter.
u32 __fastcall Ranges_LookupAdapter(const u8 *object, void *, u32 key);

// 0x6bc040 (thiscall; mask, out, pops 8): whether the mask has a bit in common with the word at +0x18 of what the dword at
// +8 points to (*out = the byte at +5 of what +0x4c points to) or else with the word at +0x1c (the byte at +6).
bool __fastcall Obj_ByteForMask(const u8 *object, void *, u32 mask, u32 *out);

// 0x6c3d80 (thiscall; mask, out, pops 8): like Obj_ByteForMask, but the byte is that of the object the dword at +0x4c names
// through its own dword at +0x1c and then +0x4c: *out = its byte at +5 / +6.
bool __fastcall Obj_ByteForMaskIndirect(const u8 *object, void *, u32 mask, u32 *out);

// 0x6d4890 (thiscall; status, out, index, pops 12): for an index up to the dword at +0xc (unsigned) the slot 4 * index - 4
// into the array at +0x14, filled on first use with the dword at +8 plus the offset it holds at 4 * index, is read into
// *out and *status = 0; else *status = -1. Returns status.
u32 *__fastcall Slots_Resolve(const u8 *object, void *, u32 *status, u32 *out, u32 index);

// 0x6b0580 (thiscall; out, index, pops 8): when the dword at +8 is not 0 and the low 16 bits of the index are within the
// word at +6 of what +0xc points to: out[0] = the word at +0x16 + (the dword the table at +0x10 holds for the index) of
// the dword at +8, out[1] = the byte at +5 of the +0xc record; else both 0. Returns out.
u32 *__fastcall Entry_Lookup(const u8 *object, void *, u32 *out, u32 index);

// 0x6ad730 / 0x6ad760 / 0x6ad790 (thiscall; status, pops 4): when the dword at +4 is not 0 (and, for the first two, bit 0 / bit 3 of
// the dword at +8 is set): +0xc = 0 / +0x10 = 1 / +0x10 = 0 and *status = 0; else *status = -1. Returns status.
u32 *__fastcall Obj_ResetC(u8 *object, void *, u32 *status);
u32 *__fastcall Obj_SetOn10(u8 *object, void *, u32 *status);
u32 *__fastcall Obj_SetOff10(u8 *object, void *, u32 *status);

// 0x6adf60 (thiscall; status, kind, pops 8): a kind from 0 to 2 is stored at +0xa4; *status = -1 whatever. Returns status.
u32 *__fastcall Obj_SetKindA4(u8 *object, void *, u32 *status, s32 kind);

// 0x6afc60 (stdcall; record, pops 4): the word at +4 of the record, or 6 for none.
u32 __stdcall Record_Word4OrSix(const u8 *record);

// 0x6d22d0 (thiscall; flags, pops 4): a bit set built from the flags: 0x1000, 0x100 unless the dword at +0xc is 11025, 1 for
// bit 12 without bit 0, 2 for bit 2, 8 for bit 3, 4 for bit 4 and 0x10 for bit 6.
u32 __fastcall Obj_RemapFlags(const u8 *object, void *, u32 flags);

// 0x6b9b30 (thiscall; n, pops 4): moves a deque-of-dwords iterator (+0 the block's first byte, +4 its end, +8 the
// position, +0xc the block's slot in the map) on by n dwords, across blocks of 0x400 dwords. Undefined return.
void __fastcall DequeIter_Advance(u8 *it, void *, s32 n);

}  // extern "C"
