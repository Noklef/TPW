// The firing of an action table's handlers: reimplementation of 0x40c900 and 0x40c990. Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x40c900 / 0x40c990 (thiscall on {entries, count (word at +4), enabled (word at +6)}; a, b, pops 8): 0 unless enabled. Otherwise the first of the 0x14-byte entries (count
// of them, signed) whose words at +2 and +4 are the low words of a and b has its handler (a pointer at +0x10 / +0xc; called with ECX = the pointer, no arguments) called when
// there is one, and its word at +6 set to 1 / 0. Returns what the handler returned (0 for none, or no entry).
u32 __fastcall Actions_FireDown(u8 *table, void *, u32 a, u32 b);
u32 __fastcall Actions_FireUp(u8 *table, void *, u32 a, u32 b);

}  // extern "C"
