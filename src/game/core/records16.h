// Helpers over arrays of 16-byte records whose first word is a 16-bit character (the text layout code of the language module):
// reimplementation of 0x6ae2e0, 0x6ae300, 0x6ae390, 0x6ae3d0 and 0x6ae440. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6ae2e0 (cdecl; char): whether the 16-bit character is a space (0x20) or a comma (0x2c).
bool __cdecl Char_IsSpaceOrComma(const u16 *c);

// 0x6ae300 (cdecl; first, last): takes the bracket class (Char_BracketClass) of the first record and finds the record from there
// on where the running count (+1 for the same class, -1 for any other non-zero class) comes back to 0; swaps those two records. Does
// nothing for an empty range or when it does not come back to 0. Undefined return.
void __cdecl Records16_SwapMatchingBracket(u8 *first, u8 *last);

// 0x6ae390 (cdecl; first, last): the first record from first up to last that is a space or comma (Char_IsSpaceOrComma), else last.
u8 *__cdecl Records16_FindSpace(u8 *first, u8 *last);

// 0x6ae3d0 (cdecl; first, last): the first record that is not a space or comma, when first is no more than two records before last
// (last - 0x20 or below), else 0.
u8 *__cdecl Records16_FindNonSpace(u8 *first, u8 *last);

// 0x6ae440 (cdecl; first, last): reverses the order of the records from first up to last.
void __cdecl Records16_Reverse(u8 *first, u8 *last);

// 0x6ae4b0 (cdecl; first, last): reorders the records of a line of text for right to left reading when any of them is in the
// range 0x5b1-0x5ea (Records16_AnyInRange5b0): the trailing records whose byte at +4 has bit 6 set are left out, the rest
// reversed (Records16_Reverse), then each run from a non-space (Records16_FindNonSpace) to the next space (Records16_FindSpace)
// is reversed back, or, when it has such a record in it, has its bracket pair swapped when it starts with an opening bracket
// (Char_BracketClass above 0). Undefined return.
void __cdecl Records16_ReorderRtl(u8 *first, u8 *last);

}  // extern "C"
