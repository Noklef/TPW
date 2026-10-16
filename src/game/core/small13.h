// A counted list range erase and nth erase, a table lookup with a limit and an activity check: reimplementation of 0x5cdfe0, 0x59c570,
// 0x5b6230 and 0x5cd660. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5cdfe0 (thiscall; out, first, last, pops 12): like List_EraseRangeCounted (small11) but the counted buffer is freed 1 byte below the pointer
// (the count is the block's first byte). Returns out.
u32 *__fastcall List_EraseRangeCounted1(u8 *object, void *, u32 *out, u8 *first, u8 *last);

// 0x59c570 (thiscall; id, limit, pops 8): the record of the table at 0x76e300 for id (see Table76e300_FindRecord) gives an index (its dword at +8; 10 for
// no record); for 10 the result is 0, otherwise the dword at 0x2c + 12 * index of the object must not be 0 and the signed limit must be at least that
// (1 or 0, as AL).
u8 __fastcall Table76e300_LimitCheck(const u8 *object, void *, u32 id, s32 limit);

// 0x5b6230 (thiscall): 1 when any of the dwords at +0 and +0x6c, the dword at +0xc of each of the nine blocks pointed to from +0xe0..+0x100, or the
// dwords at +0x324 and +0x38c is not 0 (in that order), otherwise 0.
u32 __fastcall Obj_HasActivity(const u8 *object);

// 0x5cd660 (thiscall; index, pops 4): the index-th node of the circular list at +4 (count at +8) is unlinked, its counted buffer (the block freed 1 byte
// below the pointer) released and its fields +0xc, +0x10 and +0x14 cleared, and the node freed. Returns 1, or 0 when the list is shorter (AL).
u8 __fastcall List_EraseNthCounted(u8 *object, void *, u32 index);

}  // extern "C"
