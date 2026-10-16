// Table lookups and the cursors over packed data: reimplementation of 0x6b0620, 0x6b0650, 0x6b0510, 0x6b5450, 0x6b54c0 and
// 0x6b54f0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b0620 / 0x6b0650 (thiscall; index, pops 4): when the dword at +8 (the base) is not 0 and the low 16 bits of the index are
// within the word at +6 of what +0xc points to: the dword the table at +0x10 holds for the index plus the base plus 0x18 /
// plus the base; else 0.
u32 __fastcall Table_AddressPlus18(const u8 *object, void *, u32 index);
u32 __fastcall Table_Address(const u8 *object, void *, u32 index);

// 0x6b0510 (thiscall; cursor, pops 4): reads a value from the bytes *cursor points to: the first byte less one (16 bits) when
// that is below the byte at +8 of what +0x14 points to, else it takes the byte after as well, so that (first - 1 + 0x101) * 255
// - limit * 254 + second. *cursor moves past what was read. Only the low 16 bits of the return are defined.
u16 __fastcall Packed_Read(const u8 *object, void *, const u8 **cursor);

// 0x6b5450 (thiscall; nibble, pops 4): a nibble writer (+0 = the byte, +4 = whether the next goes to the low half, +5 = a
// skip flag, +8 = the count): the nibble is put in the high half of a new byte or ORed into the low half of the current (which
// moves on); the half flips and the count is incremented. Returns the count.
u32 __fastcall Nibble_Write(u8 *object, void *, u32 nibble);

// 0x6b54c0 (thiscall; out, pops 8): the state of the nibble cursor (+0 = whether the next is the low half, +4 = the byte) is
// copied to out (out+0 = the flag, out+4 = the byte) and then moves: the byte on when the flag was set, the flag flipped.
// Returns out.
u8 *__fastcall Nibble_Next(u8 *object, void *, u8 *out, u32 unused);  // (pops 8: its callers push a second, unused word)

// 0x6b54f0 (thiscall; out, pops 8): the same for a bit cursor (+0 = the mask, +4 = the byte): the old state to out, then the
// mask rotated right by one, the byte moving on when the mask becomes 0x80. Returns out.
u8 *__fastcall Bit_Next(u8 *object, void *, u8 *out, u32 unused);

}  // extern "C"
