// Counted text buffers released, a pointer array's removal by value and a list's range erase: reimplementation of 0x47b5b0, 0x4f23f0,
// 0x47db20, 0x56b5c0 and 0x5ce540. Named by what they do until the classes are known.
//
// A counted buffer is a pointer to bytes with a use count in the byte before them (0 and 0xff mean "last user") and one more byte
// before that; the block that is freed starts 2 bytes below the pointer.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x47b5b0 (stdcall; first, last): for each 16-byte entry from first up to (not including) last: the counted buffer at +4 is released
// (one use less, or the block freed when the count is 0 or 0xff), then the dwords at +4, +8 and +0xc = 0. Nothing when first == last.
void __stdcall CountedEntries_Release(u8 *first, u8 *last);

// 0x4f23f0 (thiscall; release, pops 4): when the byte release is not 0 the counted buffer at +4 (when set) is released; then the dwords at
// +4, +8 and +0xc = 0.
void __fastcall CountedBuf_Reset(u8 *object, void *, u32 release);

// 0x47db20 (thiscall; flags, pops 4): the counted buffer at +4 (when set) is released; the dwords at +4, +8 and +0xc = 0; when bit 0 of
// the flags is set the object is freed. Returns the object.
u8 *__fastcall CountedBuf_Deleter(u8 *object, void *, u32 flags);

// 0x56b5c0 (thiscall; item, pops 4): the item is looked for in the array of pointers at +8 (count at +0x1c) from the back; when found it is
// freed, the count is one less, the slot gets the last element and the dword at +4 of what +4 points to = the new count. Returns 1 when
// found, otherwise 0.
u32 __fastcall PtrArray_RemoveAndFree(u8 *object, void *, u32 item);

// 0x5ce540 (thiscall; out, first, last, pops 12): erases the nodes of the circular list from first up to last (each unlinked and freed, the
// count at +8 one less each), stores last-reached first (always last) through out. Returns out.
u32 *__fastcall List_EraseRange(u8 *object, void *, u32 *out, u8 *first, u8 *last);

}  // extern "C"
