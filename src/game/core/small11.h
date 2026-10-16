// A slot table and a counted list erase: reimplementation of 0x4d27a0, 0x503a20 and 0x4f3400. Named by what they do until the classes are
// known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d27a0 (thiscall; id, amount, kind, extra, pops 16): the selected slot (the index at +0x3cc, within 0..19, of the 48-byte slots from +0xc)
// is checked: when the byte at +0x3d9 is not 0, the slot's dword at +0 equals id, its word at +0xc equals the low 16 bits of kind, and the word at
// +0x3e0 is 0 or equal to the low 16 bits of extra, the dword at +0x3dc gains amount. Undefined return.
void __fastcall Slots_AddIfMatch(u8 *object, void *, u32 id, u32 amount, u32 kind, u32 extra);

// 0x503a20 (thiscall; slot, pops 4): the entries (20 bytes from +0xc) are chosen by the dwords at +0x1414 + 16 * slot (0 when below 0): for slot 4
// the dword at +0x1ac of the object definition (word +0xc of the entry; ObjectDefs_Find) at 0x40 * (the byte at +0x15) + 0x1b8; for the others
// the dword at +4 of the object control record (the entry's word at +0xc; the game's table, ObjectControls_FindUncached quiet).
u32 __fastcall Slots_GetValue(const u8 *object, void *, u32 slot);

// 0x4f3400 (thiscall; out, first, last, pops 12): like List_EraseRange (small6) for a list whose nodes hold a counted buffer (see small6) at +0xc
// (released like CountedEntries_Release does, the block freed 2 bytes below the pointer) and the fields +0xc, +0x10 and +0x14 are cleared before
// the node is freed. Returns out.
u32 *__fastcall List_EraseRangeCounted(u8 *object, void *, u32 *out, u8 *first, u8 *last);

}  // extern "C"
