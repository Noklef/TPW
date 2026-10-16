// A counted list erase, a person match, a setting switch, a push into four slots, two table lookups and a list clear: reimplementation of 0x4f54e0,
// 0x5065d0, 0x550d80, 0x5633e0, 0x59c390, 0x59c410 and 0x5b15a0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4f54e0 (thiscall; out, node, pops 8): like List_EraseNodeCounted (small10) but the counted buffer's block is freed 2 bytes below the pointer.
u32 *__fastcall List_EraseNodeCounted2(u8 *object, void *, u32 *out, u8 *node);

// 0x5065d0 (thiscall; word, pops 4): Person_HasWord (small9); when it holds: 1 for kind 5 with the dword at +0x19c equal to 0xb and for kind 4 with
// it equal to 0xd, otherwise 0.
u32 __fastcall Person_HasWordInState(const u8 *object, void *, u32 word);

// 0x550d80 (thiscall; mode, pops 4): the dword at +0 = mode (the original reports mode >= 3 to its empty debug printf); for 0 / 1 / 2 the bits 23..25
// of the dword at 0x7a1a8c are set to 1 / 2 / 4 (bit 23 / 24 / 25 alone: the other two cleared); other modes leave it.
void __fastcall Setting_SetMode(u8 *object, void *, u32 mode);

// 0x550ca0 (thiscall): the mode is read from the bits 23..25 of the dword at 0x7a1a8c (the highest set of 25 / 24 / 23 gives 2 / 1 / 0; none gives 0), stored
// at +0 and the bits set to that single one (as Setting_SetMode does). Returns the object.
u8 *__fastcall Setting_ReadMode(u8 *object);

// 0x4c9050 (cdecl on the game object [0x80239c]): whether the dword at +0x1da710 of the game object is 0 (as AL).
u8 __cdecl Game_Field1da710IsZero();

// 0x5633e0 (stdcall; source, unused, unused, object, pops 16): when the count at +0x788 is below 4 the next 16-byte slot from +0x448 gets the four
// dwords from source (zeros for 0) and the count rises. Returns 1.
u32 __stdcall Quad_Push(const u32 *source, u32, u32, u8 *object);

// 0x59c390 (thiscall; id, pops 4): the record of the table at 0x76e300 for id (see Table76e300_GetByte: sic, the record at the id's own slot even
// when the search found the id elsewhere): the dword at 0x24 + 12 * (the record's dword at +8) of the object; 0 for no record.
u32 __fastcall Table76e300_GetObjectDword(const u8 *object, void *, u32 id);

// 0x59c410 (thiscall; id, pops 4): the same with the dword at 0x28 + 12 * (...) being not 0 (as AL; 0 for no record).
u8 __fastcall Table76e300_ObjectDwordSet(const u8 *object, void *, u32 id);

// 0x5b15a0 (thiscall): every node of the circular list at +4 (count at +8) is unlinked, its counted buffer at +8 reset (CountedBuf_Reset, releasing)
// and the node freed; then the sentinel is freed and the dwords at +4 and +8 = 0.
void __fastcall CountedList_Clear(u8 *object);

// The record of the table at 0x76e300 that the lookups above use (not a game function): see Table76e300_GetObjectDword.
u32 Table76e300_FindRecord(u32 id);

}  // extern "C"
